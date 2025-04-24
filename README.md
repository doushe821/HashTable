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

1. Number of buckets is much less than number of keys, 1024 in our case (this way, with our test files Load factor will be around 10 word per bucket).
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

## Hash comparison
In the beginning, we have to choose right hash: one that can easily be optimized, yet one with good distribution.

I came up with a simple cash that calculates subsumms of the key (in groups of 8) and then xors them with seed.

Let's compare its distribution with MurMur2 (which is fairly harder to optimize):

![](HashTestImages/MurMur1024.png)

![](HashTestImages/Whacky1024.png)

As you can see, distributions are comparable, however, Whacky hash loses a bit. To measure it properly, we will calculate dispersion of number of words in each bucket (normalised by load factor):

$\sigma_\text{MurMur}=1.0234509874327002$

$\sigma_\text{Whacky}=1.7210710632676653$

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

That way we can tell that we should optimize hash first.

## First optimization
Program hash function (WackyHash2) calculates subsums of the key and then xors them with seed.

We can easily optimize it with intrinsics functions that use SIMD instructions:

<details>
<summary>Show/hide code</summary>
  
```c

static size_t WhackyHash2_SIMD(void* Key, size_t len, size_t MaxValue)
{
    size_t seed = 0x69696ABE1;
    size_t MLG = 0xABE1;
    size_t tony = 0xEDA666EDA6667878;
    
    __m256i seedm256 = _mm256_set1_epi64x(seed);

    size_t HashSubSums[4] = {};
    for(size_t i = 0; i < 4; i++)
    {
        HashSubSums[i] = ((char*)Key)[i * 8] +  ((char*)Key)[i * 8 + 1] + ((char*)Key)[i * 8 + 2] + ((char*)Key)[i * 8 + 3]
        + ((char*)Key)[i * 8 + 4] +  ((char*)Key)[i * 8 + 5] + ((char*)Key)[i * 8 + 6] + ((char*)Key)[i * 8 + 7];
    }

    __m256i SubSums256 = {(long long)HashSubSums[0], (long long)HashSubSums[1], (long long)HashSubSums[2], (long long)HashSubSums[3]};
    SubSums256 = _mm256_xor_si256(SubSums256, seedm256);

    size_t HashSum = SubSums256[0] + SubSums256[1] + SubSums256[2] + SubSums256[3];
    return HashSum % MaxValue;
}

```
</details>

This way, we can gain high performance boost using only 3 assembly lines. Quite impressive!

This is profile of first optmization:

<details>
  <summary>First optimization profile</summary>
  
  ![](PerfImages/FirstOptProfile.png)

</details>


## Second optimization
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
<summary>Second optimization profile</summary>

  ![](PerfImages/SecondOptProfile.png)

</details>

It is obvious that next step is optimizing SimpleHash() - programms' hash function.

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
    <th>$14402605506.0\pm 33491079$</th>
    <th>$0.002$</th>
    <th>$1.00$</th>
    <th>$0$</th>
  </tr>
  <tr>
    <th>First optimization</th>
    <th>$10651982323\pm 29034815$</th>
    <th>$0.003$</th>
    <th>$1.36$</th>
    <th>$3$</th>
  </tr>
  <tr>
    <th>Second optimization</th>
    <th>$7040251868.0\pm 5638961$</th>
    <th>$0.0008$</th>
    <th>$2.06$</th>
    <th>$40$</th>
  <\tr>
  <tr>
    <th>Third optimization</th>
    <th>$6876360513.6\pm 4891910$</th>
    <th>$0.0007$</th>
    <th>$2.12$</th>
    <th>$5$</th>
  </tr>
</table>
</details>

Now let's compare effectiveness of every optimization:

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
    <th>$120$</th>
  </tr>
  <tr>
    <th>Second optimization</th>
    <th>$17.5$</th>
  </tr>
  <tr>
    <th>Third optimization</th>
    <th>$12$</th>
  </tr>
</table>

Now let's calculate overall COP:

```math
\eta=23.3
```
(result are given without absolute errors because they are negligible)

## Sufficiency
Now, why would we stop on the third optimization? Dynamic shows, that optimizing hottest function at that point gives less than $1\%$ to performance, so, considering we cannot optimize other functions any further,
we come to a conclusion: any further optimization are gonna be insufficient.

## Conclusion
As expected, most efficient optimizations in terms of COP was the first one, followed by the second. However, it is noticable that second optimizatin gave more overall performance boost. 
