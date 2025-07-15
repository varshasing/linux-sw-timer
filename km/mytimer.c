/* Necessary includes for device drivers */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/proc_fs.h>
#include <linux/fcntl.h>
#include <linux/uaccess.h>
#include <linux/sched.h>
#include <linux/jiffies.h>
#include <linux/ctype.h>
#include <linux/list.h>

MODULE_LICENSE("Dual BSD/GPL");

struct timer_entry {
    char message[129];
    struct timer_list ktimer;
    struct list_head list;
};

/* Declaration of global variables */
static LIST_HEAD(timer_list);
static int timerMajor = 61;
static int timerCount = 0;
static int timerCapacity = 1;
static char *userInput;
static char *userOutput;
static unsigned bite = 256;
const int inputSize = 256;
const int outputSize = 1024;

/* Declaration of functions */
static int timerInit(void);
static void timerExit(void);
static int timerOpen(struct inode* inode, struct file* filp);
static ssize_t timerRead(struct file* filp, char* buf, size_t count, loff_t* f_pos);
static ssize_t timerWrite(struct file* filp, const char* buf, size_t count, loff_t* f_pos);
static int timerRelease(struct inode* inode, struct file* filp);
static void createTimer(unsigned long seconds, char* message);
static void exitTimer(struct timer_list* ktimer);
static void listActiveTimers(void);
static void changeMax(unsigned long newMax);

/* Declaraction of usual file access functions */
struct file_operations timerFileOps = {
    .read = timerRead,
    .write = timerWrite,
    .open = timerOpen,
    .release = timerRelease
};

/* declaration of init and exit functions */
module_init(timerInit);
module_exit(timerExit);

/* Initalize the timer upon insmod */
static int timerInit(void) {
    int result = register_chrdev(timerMajor, "mytimer", &timerFileOps);
    if (result < 0) {
        printk(KERN_ALERT "mytimer: Cannot obtain major number %d\n", timerMajor);
        return result;
    }
    userInput = kmalloc(inputSize, GFP_KERNEL);
    userOutput = kmalloc(outputSize, GFP_KERNEL);
    if (!userInput || !userOutput) {
        printk(KERN_ALERT "mytimer: Memory allocation failed\n");
        kfree(userInput);
        kfree(userOutput);
        return -ENOMEM;
    }
    memset(userInput, 0, inputSize);
    memset(userOutput, 0, outputSize);
    printk(KERN_ALERT "Inserting mytimer module\n");
    return 0;
}

/* exit the timer and free memory */
static void timerExit(void) {
    struct timer_entry *entry, *tmp;
    unregister_chrdev(timerMajor, "mytimer");
    kfree(userInput);
    kfree(userOutput);
    list_for_each_entry_safe(entry, tmp, &timer_list, list) {
        del_timer_sync(&entry->ktimer);
        list_del(&entry->list);
        kfree(entry);
    }
    printk(KERN_ALERT "Removing mytimer module\n");
}

/* timer expires */
static void exitTimer(struct timer_list* ktimer) {
    struct timer_entry *entry;
    list_for_each_entry(entry, &timer_list, list) {
        if (&entry->ktimer == ktimer) {
            pr_info("%s\n", entry->message);
            list_del(&entry->list);
            kfree(entry);
            timerCount--;
            return;
        }
    }
}

/* Used with -s flag, adds a new timer if space or modifies a timer */
static void createTimer(unsigned long seconds, char* message) {
    struct timer_entry *new_timer;

    char trimmedMessage[129];
    char *start = message;
    char* end;

    /* trim whitespace */
    while(*start == ' ')
    {
      start++;
    }

    end = start + strlen(start) - 1;
    while (end > start && (*end == ' ' || *end == '\n'))
    {
      *end-- = '\0';
    }

    strcpy(trimmedMessage, start);

    /* check for existing timer */
    list_for_each_entry(new_timer, &timer_list, list) {
      if(strcmp(trimmedMessage, new_timer->message) == 0)
      {
        mod_timer(&new_timer->ktimer, jiffies + msecs_to_jiffies(seconds * 1000));
        sprintf(userOutput, "The timer %s was updated!\n", message);
        return;
      }
    }

    /* limit */
    if (timerCount >= timerCapacity) {
        sprintf(userOutput,"%d timer(s) already exist(s)!\n", timerCount);
        return;
    }

    /* set new timer, jiffies conversion */
    strcpy(userOutput, "");
    new_timer = kmalloc(sizeof(*new_timer), GFP_KERNEL);
    if (!new_timer) return;
    strcpy(new_timer->message, trimmedMessage);
    timer_setup(&new_timer->ktimer, exitTimer, 0);
    mod_timer(&new_timer->ktimer, jiffies + msecs_to_jiffies(seconds * 1000));
    list_add_tail(&new_timer->list, &timer_list);
    timerCount++;
}

/* list all timers for -l option */
static void listActiveTimers(void) {
    struct timer_entry *entry;
    char *output = userOutput;
    list_for_each_entry(entry, &timer_list, list) {
        unsigned long timeLeft = jiffies_to_msecs(entry->ktimer.expires - jiffies) / 1000;
        output += sprintf(output, "%s %ld\n", entry->message, timeLeft);
    }
}

/* changing the number of timers */
static void changeMax(unsigned long newMax) {
    struct timer_entry *entry, *tmp;
    list_for_each_entry_safe(entry, tmp, &timer_list, list) {
        if (timerCount > newMax) {
            del_timer_sync(&entry->ktimer);
            list_del(&entry->list);
            kfree(entry);
            timerCount--;
        }
    }
    timerCapacity = newMax;
}

/* from nibbler, for opening */
static int timerOpen(struct inode *inode, struct file *filp) {
    return 0;
}

/* from nibbler, reading  */
static ssize_t timerRead(struct file *filp, char *buf, size_t count, loff_t *f_pos) {
    unsigned int timerLen = strlen(userOutput);
    if (*f_pos >= timerLen) return 0;
    if (count > timerLen - *f_pos) count = timerLen - *f_pos;
    if (copy_to_user(buf, userOutput + *f_pos, count)) return -EFAULT;
    strcpy(userOutput, "");
    return count;
}

/* user space has written to shared file, need to modify timers  */
static ssize_t timerWrite(struct file *filp, const char *buf, size_t count, loff_t *f_pos) {
    char *cmdBuffer = kmalloc(256, GFP_KERNEL);
    unsigned long seconds = 0, newMax = 0;
    char *message, *secString;

    /* init for input */
    memset(userInput, 0, inputSize);
    if (copy_from_user(userInput, buf, count)) return -EFAULT;

    /* check for flags */
    /* FLAG -s */
    if (strncmp(userInput, "-s", 2) == 0) {
        strcpy(cmdBuffer, userInput + 3);
        secString = strsep(&cmdBuffer, " ");
        message = strsep(&cmdBuffer, "");
        seconds = simple_strtoul(secString, NULL, 10);
        createTimer(seconds, message);
    /* FLAG -l */
    } else if (strncmp(userInput, "-l", 2) == 0) {
        listActiveTimers();
    /* FLAG -m */
    } else if (strncmp(userInput, "-m", 2) == 0) {
        newMax = simple_strtoul(userInput + 3, NULL, 10);
        changeMax(newMax);
    }

    /* free buffer */
    kfree(cmdBuffer);
    return count;
}

/* from nibbler */
static int timerRelease(struct inode* inode, struct file* filp) {
    return 0;
}

