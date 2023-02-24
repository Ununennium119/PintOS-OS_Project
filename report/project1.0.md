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

۷.

۸.

۹.

۱۰.

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