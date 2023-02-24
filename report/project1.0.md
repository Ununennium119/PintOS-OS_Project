تمرین گروهی ۱/۰ - آشنایی با pintos
======================

شماره گروه:
-----
> نام و آدرس پست الکترونیکی اعضای گروه را در این قسمت بنویسید.

محمد مهدی صادقی moh02sadeghi@gmail.com

آرش یادگاری arashyadegari0402@gmail.com

محمد خسروی mohamad138121@gmail.com

عرفان مجیبی mojibierfan@gmail.com

مقدمات
----------
> اگر نکات اضافه‌ای در مورد تمرین یا برای دستیاران آموزشی دارید در این قسمت بنویسید.


> لطفا در این قسمت تمامی منابعی (غیر از مستندات Pintos، اسلاید‌ها و دیگر منابع  درس) را که برای تمرین از آن‌ها استفاده کرده‌اید در این قسمت بنویسید.

آشنایی با pintos
============
>  در مستند تمرین گروهی ۱۹ سوال مطرح شده است. پاسخ آن ها را در زیر بنویسید.


## یافتن دستور معیوب

۱.
0xc0000008

۲.
0x8048757

۳.
command:

```objdump -d  do-nothing```

function name: `_start`

instruction: `mov    0x24(%esp),%eax`



۴.
```C
void
_start (int argc, char *argv[])
{
  exit (main (argc, argv));
}
```

`sub    $0x1c,%esp` : allocate 28 bytes from stack

`mov    0x24(%esp),%eax`: move the value at address [%esp] + 36 to %eax

`mov    %eax,0x4(%esp)`: move %eax to 4 + [%esp]

`mov    0x20(%esp),%eax`:  move the value at address [%esp] + 32 to %eax

`mov    %eax,(%esp)`: move %eax to [%esp]

`call   80480a0 <main>`: call main

`mov    %eax,(%esp)`: move %eax to [%esp]

`call   804a2bc <exit>`: call exit

در واقع پارامترهای تابع را در استک پوش می‌کند و با مقدار خروجی تابع main تابع exit را صدا می‌زند.


۵.
هدف خواندن ورودی‌های تابع مذکور بوده است، که در استک وجود ندارد و باعث شده که به قسمتی از حافظه اشاره کند که برای استک نیست.

## به سوی crash


۶.
name = `main`

address: 0xc000e000

other thread: `idle`

```
pintos-debug: dumplist #0: 0xc000e000 {tid = 1, status = THREAD_RUNNING, name = "main", '\000' <repeats 11 times>, stack = 0xc000edec <incomplete sequence \357>, priority = 31, allelem = {prev = 0xc0035910 <all_list>, next = 0xc0104020}, elem = {prev = 0xc0035920 <ready_list>, next = 0xc0035928 <ready_list+8>}, pagedir = 0x0, magic = 3446325067}
pintos-debug: dumplist #1: 0xc0104000 {tid = 2, status = THREAD_BLOCKED, name = "idle", '\000' <repeats 11 times>, stack = 0xc0104f34 "", priority = 0, allelem = {prev = 0xc000e020, next = 0xc0035918 <all_list+8>}, elem = {prev = 0xc0035920 <ready_list>, next = 0xc0035928 <ready_list+8>}, pagedir = 0x0, magic = 3446325067}
```

۷.

```
#0  process_execute (file_name=file_name@entry=0xc0007d50 "do-nothing") at ../../userprog/process.c:32
#1  0xc0020268 in run_task (argv=0xc00357cc <argv+12>) at ../../threads/init.c:288
#2  0xc0020921 in run_actions (argv=0xc00357cc <argv+12>) at ../../threads/init.c:340
#3  main () at ../../threads/init.c:133
```

```C
init.c:133: run_actions (argv);
***
init.c:340: a->function (argv);
***
init.c:288: process_wait (process_execute (task));
```

۸.

```
pintos-debug: dumplist #0: 0xc000e000 {tid = 1, status = THREAD_BLOCKED, name = "main", '\000' <repeats 11 times>, stack = 0xc000eeac "\001", priority = 31, allelem = {prev = 0xc0035910 <all_list>, next = 0xc0104020}, elem = {prev = 0xc0037314 <temporary+4>, next = 0xc003731c <temporary+12>}, pagedir = 0x0, magic = 3446325067}
pintos-debug: dumplist #1: 0xc0104000 {tid = 2, status = THREAD_BLOCKED, name = "idle", '\000' <repeats 11 times>, stack = 0xc0104f34 "", priority = 0, allelem = {prev = 0xc000e020, next = 0xc010a020}, elem = {prev = 0xc0035920 <ready_list>, next = 0xc0035928 <ready_list+8>}, pagedir = 0x0, magic = 3446325067}
pintos-debug: dumplist #2: 0xc010a000 {tid = 3, status = THREAD_RUNNING, name = "do-nothing\000\000\000\000\000", stack = 0xc010afd4 "", priority = 31, allelem = {prev = 0xc0104020, next = 0xc0035918 <all_list+8>}, elem = {prev = 0xc0035920 <ready_list>, next = 0xc0035928 <ready_list+8>}, pagedir = 0x0, magic = 3446325067}
```

۹.

process.c:45:
```C
tid = thread_create (file_name, PRI_DEFAULT, start_process, fn_copy);
```

۱۰.

```
$1 = {edi = 0x0, esi = 0x0, ebp = 0x0, esp_dummy = 0x0, ebx = 0x0, edx = 0x0, ecx = 0x0, eax = 0x0, gs = 0x23, fs = 0x23, es = 0x23, ds = 0x23, vec_no = 0x0, error_code = 0x0, frame_pointer = 0x0, eip = 0x0,
cs = 0x1b, eflags = 0x202, esp = 0x0, ss = 0x23}
```

۱۱.

۱۲.

۱۳.


## دیباگ

۱۴.

۱۵.

۱۶.

۱۷.

۱۸.

۱۹.