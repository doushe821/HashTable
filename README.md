# Optimized Hash Table
This is an open adressing hash table implementation with assembly optimizations. This is an educational project, main purpose of which is to get familiar with ways of optimizing data structures and even some functions from libc via assembly inline 
code, SIMD instructions and whole functions written in assembly.
## Info
Hash functionis a function that maps arbitrary-size data to a fixed-size data, e.g. strings to integer numbers.
Hash function's effectiveness is defined by how evenly it maps output range. Ideally, every hash value from output range should have same probability.

Hash table with open adressing is a closed data structure which consists of buckets (lists with data) and dictionary. Each bucket has its own unique number that is defined by 
hash sum of elements that it contains. Each key has its own value, which can be accessed. Each bucket contains list of keys (because several keys can have same hash sum) and list of values (they can be in the same list or in two separate,
they have to be synchronized, though). 

![](https://upload.wikimedia.org/wikipedia/commons/thumb/9/90/HASHTB12.svg/1920px-HASHTB12.svg.png)

Generally, effectiveness of a hash table is defined by number of buckets it has: if there is a bucket for every element, values can be accessed in constant time, because there is gonna be a bijection between
hash sums and keys, hence, hash sums and values. 

In modern hash tables buckets contain, on average, 1.5 elements, so  they can be organised not as lists, but as simple arrays. However, it is really hard to optimize such hash table, so, for educational purposes 
we will downgrade our hash table: it will be using only 256 buckets, which that if we would take a huge amount of keys, some of them definetely will fall into the same bucket. That downgrade will allow us to look 
for some optimizations.

This project main idea is to implement some low-level optimization such as assembly inline code, SIMD, assembly functions in open adressing hash table structure. To implement those optimizations 
we need to make our task less general and specify format and funtionality of our hash table. Those restrictions are gonna be:

1. Number of buckets is much less than number of keys, 256 in our case, while.
2. All compiler optimizations are gonna be disabled, programm will be running with O0.
3. Hash function is going to be extremely simple.
4. We are going to work only with ywords (arrays of linear data with maximum length of 256 bits). For better visualization we are going to use char strings, even tho our table will be able to work with any type of data within size limit.

## Low-level optimizations problems
Main problem of any low-level optimizations is that it reduces readability and compatability of program: it becomes harder to read, loses some functionality, because while implementing low-level optimizations, 
we always rely on opportunities that are specific to a task, e.g. strictly limited key size. 

Because of that we should be conscious with our optimizations, that is why they should be precise and laconic.

## Profiling
Profiling is measuring programm's space and time usage function-wise. It can help to detect so cold "hot" functions: ones that are working most of the time. This will allow us to targetly
optimize those functions and achieve bigger performance boost. It is mandatory to keep our assembly insertions as brief as possible.

Profiler for this project is perf. Perf is a part of Linux kernel, so it can be used on any distro and is well accessible. Besides functions' time usage, perf also shows how many ticks it took to finish program, so basically we are gonna be measuring
time with help of perf.

## Measurements
Completion time of programm is measured by perf. It will be measured for each version of programm until random error is less than 1% of measuring value.

Since we are going to implement optimizations subsequently, we have to somehow measure, how effective each optimization was. In order to do that we will use equivalent of COP (coefficient of performance) that also takes in account 
quantity of lines written in assembly, or using intrinsics functions:
```math
\eta=\frac{k}{q}\cdot1000
```
Where $k=\frac{t_\text{naive}}{t_\text{optimized}}$ and q is number of strings written in assembly.

Since hash tables are mainly a searching tool, all the benches will include only repeatative searching requests.

Error is calculated using following formula:
```math
\sigma_k=k\cdot(\frac{\sum(k_i-\overline{k})^2}{N(N-1)})^{1/2}
```

## Naive version
Naive version is pretty simple: hash function calculates sum of every charachter of the key string and takes remainder from division by number of buckets (256 in our case) as a hash for bucket.

Pointers to buckets are stored in a list, and buckets themselves are initialized after first their "index" hash sum is approached for the first time.

If same word appears multiple times, its value is being increased, so at the end we will have a statistical dictionary of a book.

Searching and inserting elements in hash table is implemented by calculating hash of a given key, then, finding it with strcmp and linear search in the bucket.

Keys in values are stored in two separate lists, however, their indexes in the lists are synchronized.

This is the profile of naive version:
<details>
<summary>Naive version profile</summary>
  
![](PerfImages/NaiveProfile.png)
</details>

On the first glance, it looks like hash functions takes most of the working time, however there is a block of functions that are responsible for searching elements in the hash table and their combined time
is actually bigger, than SimpleHash() time. So we will optimize search first.

![](PerfImages/NaiveActualTopTime.png)

## First optimization
In order to optimize search function we will the fact that our keys' size is limited to 256 bits. 

New function - size_t ListSearch(const char* Key, void* listData, size_t ListSize) loads key and elements of the list (here it treats list like an array) in ymm registers and then xors them. Then, vptest allows to check for 0 in register (it sets ZF to 1, if ymm == 0). Return value is index of the key in the bucket. If key isn't found, it returns 0. 

This simple function uses SIMD instructions instead of linear byte-to-byte comparison as strncmp or strcmp does, which makes it faster.

Here is ListSearch() source code:

<details>
<summary>Show/hide code</summary>
  
```asm
global ListSearch

ListSearch:
    xor rax, rax
    cmp rdx, 0h
    je .KeyFound

    add rsi, 20h
 
.SearchLoop:

    inc rax
    vmovaps ymm0, yword [rdi]
    vmovaps ymm1, yword [rsi]

    vpxor ymm0, ymm1 
    vptest ymm0, ymm0
    jz .KeyFound

    add rsi, 20h
    sub rdx, 20h

    cmp rdx, 0h
    jne .SearchLoop

    xor rax, rax 
    ret
    

.KeyFound:
  ret
```
</details>

<details>
<summary>First optimization profile</summary>

![](PerfImages/FirstOptProfile.png)
</details>

It is obvious that next step is optimizing SimpleHash() - programms' hash function.

## Second optimization
As mentioned earlier, programm's hash function is the simplest one can come up with: it calculates sum of all keys's byte, treating them as char variables and then takes remainder of division by 256 (number of buckets). We will
write it on inline assembly, so it will always be inlined and there not gonna be anything unnecessary in it.

<details>
<summary>Show/hide code</summary>
  
```asm

asm volatile
( 
    "xor %%rax, %%rax\t\n"
    "xor %%rdx, %%rdx\t\n"

    "movq %1, %%rsi\t\n"
    "movq $32, %%rcx\t\n"

    ".HashInitLoop:\t\n"

    "movb (%%rsi), %%dl\t\n"
    "addq %%rdx, %%rax\t\n"

    "add $1, %%rsi\t\n"

    "dec %%rcx\t\n"
    "cmp $0, %%rcx\t\n"
    "jne .HashInitLoop\t\n"

    "movq %%rax, %0\t\n"
    :"=r" (hash)
    :"r" (word)
    :"rax", "rbx", "rcx", "rdx", "rsi", "memory"
);
```
</details>

This is profile of second optmization:

<details>
  <summary>Second optimization profile</summary>
  
![](PerfImages/SecondOptProfile.png)
</details>

## Third optimization

Now it's time ot optimize memset() function that is needed to clear the buffer when hash table is being intialized.

That's easy, because, again, keys' sizes are limited to 256 bits, so we can load buffer to ymm register and then set it to needed value via SIMD instructions.

<details>
<summary>Show/hide code</summary>
```asm
asm volatile
( 
    "vpxor %%ymm0, %%ymm0, %%ymm0\t\n"
    "vmovaps %%ymm0, (%0)"
    :"=r" (word)
    :"r" (word)
    :"memory", "ymm0"
);
```
</details>

<details>
  
<summary>Third optimization profile</summary>
  
![](PerfImages/ThirdOptProfile.png)
</details>

## Data processing
Let's place experimental data in a table:
<details>
  <summary>Show/hide data table</summary>
<table>
  <tr>
    <th>Version</th>
    <th>$t$</th>
    <th>$\varepsilon_t$</th>
    <th>$k$</th>
    <th>$q$</th> 
  </tr>
  <tr>
    <th>Naive</th>
    <th>$405024407\pm 3719079$</th>
    <th>$0.009$</th>
    <th>$1.0$</th>
    <th>$0$</th>
  </tr>
  <tr>
    <th>First optimization</th>
    <th>$325236470\pm 2852285$</th>
    <th>$0.009$</th>
    <th>$1.25$</th>
    <th>$31$</th>
  </tr>
  <tr>
    <th>Second optimization</th>
    <th>$(250443085\pm 1621090)$</th>
    <th>$0.006$</tr>th>
    <th>$1.62$</th>
    <th>$40$</th>
  <\tr>
  <tr>
    <th>Third optimization</th>
    <th>$(249389648\pm 1013016)$</th>
    <th>$0.004$</th>
    <th>$1.63$</th>
    <th>$5$</th>
  </tr>
</table>
</details>

Now let's compare effectiveness of every optimization:

<details>
  <summary>Show/hide data table</summary>
<table>
  <tr>
    <th>Version</th>
    <th>$\eta$</th>
  </tr>
  <tr>
    <th>First optimization</th>
    <th>$40.32$</th>
  </tr>
  <tr>
    <th>Second optimization</th>
    <th>$32.5$</th>
  </tr>
  <tr>
    <th>Third optimization</th>
    <th>$200.8$</th>
  </tr>
</table>
</details>

According to results, we can tell that the method of calculating COP that we have chosen is not the right one, since the least impactful optimization has the highest COP. Let's change our definition of COP:
```math
\eta=\frac{\Delta k}{q}\cdot 1000
```
This way we get:

<table>
  <tr>
    <th>Version</th>
    <th>$\eta$</th>
  </tr>
  <tr>
    <th>First optimization</th>
    <th>$8.06$</th>
  </tr>
  <tr>
    <th>Second optimization</th>
    <th>$9.25$</th>
  </tr>
  <tr>
    <th>Third optimization</th>
    <th>$0.8$</th>
  </tr>
</table>

Now COP correlates much more with actual data.

## Sufficiency
Now, why would we stop on the third optimization? Dynamic shows, that optimizing hottest function at that point gives less than $1\%$ to performance, so, considering we cannot optimize other functions any further,
we come to a conclusion: any further optimization are gonna be insufficient.

## Conclusion
The most impactful optimization was the second one, which was unexpected, as the first one reduced relative time of search module from $28.29\%$ to $13.12\%$.
Because of that we can assume, that we should have picked up first optimization target strictly following profile. However, this conclusion is speculative, 
since search module and hash function took almost even time, so may be our way of optimizing hash calculation was just more effective.
