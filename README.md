# HyperBloom

A collection of high performance bloom filter data structures for use in C. All filters use the 64-bit version of [Cyan4973](https://github.com/Cyan4973)'s [xxhash](https://github.com/Cyan4973/xxhash) algorithm

## What is a Bloom Filter?

<div align="center">

![Header Image](assets/header.png)

</div>

While creating an account, you want to enter a cool username, you entered it and got a message, **“Username is already taken”**. You then tried a bunch of variations, and all kept saying that the username was taken until you found one that was still available. In this process, ever wondered how the website manages to search through millions of usernames to determine if it is taken?

Here is where a Bloom filter is used.

A Bloom filter is a space-efficient _probabilistic_ data structure that is used to check whether an item is a member of a set.
The bloom filter will always say **"yes"** if an item is a member of a particular set. However, it might still say "yes" although
an item is not a part of that set (false positives).
