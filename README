Written by Varsha Singh

Linux kernel module to control a software (kernel) timer for Linux running on an ARMv7 processor.

### Kernel Module
- creates kernel (software) timers and prints a specified mssage on the coonsole when the timer expires. Note that a kernel timer is different from a hardware timer.

### Userspace Program
- Communicates with kernel module. Capable of registering at least one timer (up to five) such that a user-provided message will be printed to `tty` after a user-provided time. Only configures timer(s), actual operation of kernel timer(s) and printing of timer experiation message is handled by kernel module
Supports the following command line arguments:
- `-l`: List to `stdout` the expiration time `[SEC]` (offset in seconds from the current time) of the currently registered timer(s) and the message `[MSG]` that would be printed upon timer expiration, one line per timer.
- `-s [SEC] [MSG]`: Registers a new timer that, after `[SEC]` seconds, will print the message `[MSG]`. Does not print anything if registraton is successful. If an active timer with the same message exists, then resets that timer's remaining time to `[SEC]` and prints an updated message. Also prints error message if attempting to register more than the supported number of active timers.
- `-m [COUNT]`: Changes the number of active timers supported by kernel module to `[COUNT]`

### Resources Used
https://static.lwn.net/images/pdf/LDD3/ch07.pdf
https://static.lwn.net/images/pdf/LDD3/ch02.pdf
https://www.oreilly.com/library/view/linux-device-drivers/0596000081/ch03s02.html
https://linux.die.net/man/8/insmod
https://embetronicx.com/tutorials/linux/device-drivers/using-kernel-timer-in-linux-device-driver/#:~:text=Timer%20in%20Linux%20Kernel,for%20a%20variety%20of%20tasks.
https://stackoverflow.com/questions/942273/what-is-the-ideal-fastest-way-to-communicate-between-kernel-and-user-space

### Assumptions
Some assumptions made were those provided:
[SEC] will be an integer between 1 and 86,400 (inclusive)
[COUNT] will be an integer between 1 and 5 (inclusive)
[MSG] will be a string no longer than 128 characters. The strings with which we will test your submission contain only alphanumerics, spaces, tabs, and/or the following special characters: !@#$%^&*(),./\:

Additionally, assumes no instance to lower the number of supported active timers when there are exists more than the current active timers in use.
