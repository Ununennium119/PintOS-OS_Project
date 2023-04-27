# تمرین گروهی ۲ - گزارش

## گروه ۲

## نام و آدرس پست الکترونیکی اعضای گروه

محمد مهدی صادقی moh02sadeghi@gmail.com

آرش یادگاری arashyadegari0402@gmail.com

محمد خسروی mohamad138121@gmail.com

عرفان مجیبی mojibierfan@gmail.com

ماموریت‌ها
====================

## ساعت زنگ دار


## زمان‌بند اولویت‌دار

در طراحی نهایی، `struct` هایی که دست‌خوش تغییر شدند به صورت زیرند:

```C
thread.h:

struct thread {
    ...
    int base_priority;                  /* Base Priority */
    struct lock* lock_waiting_for;      /* Lock Thread waits on */
    struct list held_locks;             /* List of held locks */
    bool is_donated;
    ...
}

synch.h:

struct lock {
    ...
    int max_priority;           /* Priority of the highest-priority thread waiting for it. */
    struct list_elem elem;      /* List element for priority donation. */
}

synch.c:

struct semaphore_elem {
    ...
    int priority;
};

```
نسبت به داک طراحی، یک فیلد `is_donated` در استراکت ترد اضافه‌تر است که در مواقع ست کردن اولویت لازم بود بدانیم ترد فعلی در وضعیت `donated` هست یا نه.

همچنین موارد طراحی مربوط به `monitor` که با استفاده از `cond_var`پیاده می‌شود در نظر گرفته نشده بود که داخل تست‌ها بود و این مورد را با اضافه کردن یک فیلد `priority` به استراکت `semaphore_elem` هندل کردیم.


## آزمایشگاه زمان‌بندی

## وظایف افراد

قسمت آزمایشگاه زمان‌بندی توسط یک نفر از اعضا (آرش یادگاری) انجام شد؛ ساعت زنگ‌دار و زمان‌بند اولویت‌دار با همکاری ۳ نفر دیگر پیاده‌سازی شد.