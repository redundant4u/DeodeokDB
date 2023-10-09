# DeodeokDB

- Implementation of simple DB(sqlite based) using C
- Reference: [Lets' Build a Simple Database](https://cstack.github.io/db_tutorial/)

## Usage

### Build & Run

```bash
make
./main test.db
```

### Test

```bash
bundle install
make test
```

## Commands

### Select

```bash
select
(1, user1, person1@example.com)
(2, user2, person2@example.com)
(3, user3, person3@example.com)
```

### Insert

```bash
# id name email
insert 1 user user@user.com
```

### Btree

```bash
.btree
Tree:
- internal (size 1)
  - leaf (size 7)
    - 1
    - 2
    - 3
    - 4
    - 5
    - 6
    - 7
  - key 7
  - leaf (size 8)
    - 8
    - 9
    - 10
    - 11
    - 12
    - 13
    - 14
    - 15
```

### Constants

```bash
.constants
Constants:
ROW_SIZE: 293
COMMON_NODE_HEADER_SIZE: 6
LEAF_NODE_HEADER_SIZE: 14
LEAF_NODE_CELL_SIZE: 297
LEAF_NODE_SPACE_FOR_CELLS: 4082
LEAF_NODE_MAX_CELLS: 13
```

## Debugging (Visual Studio Code)

1. Install [CodeLLDB](https://marketplace.visualstudio.com/items?itemName=vadimcn.vscode-lldb) extension
2. Check `tasks.json` and `launch.json`
3. Set breakpoints
4. Run(F5)

## Update

- 231009: [Splitting Internal Nodes](https://cstack.github.io/db_tutorial/parts/part14.html)
- 231007: [Updating Parent Node After a Split](https://cstack.github.io/db_tutorial/parts/part13.html)
- 231004: [Scanning a Multi-Level B-Tree](https://cstack.github.io/db_tutorial/parts/part12.html)
- 231004: [Recursively Searching the B-Tree](https://cstack.github.io/db_tutorial/parts/part11.html)
- 231003: [Splitting a Leaf Node](https://cstack.github.io/db_tutorial/parts/part10.html)
- 231002: [Binary Search and Duplicate Keys](https://cstack.github.io/db_tutorial/parts/part9.html)
- 231001: [B-Tree Leaf Node Format](https://cstack.github.io/db_tutorial/parts/part8.html)
- 230930: [The Cursor Abstraction](https://cstack.github.io/db_tutorial/parts/part6.html)
- 230929: [Persistence to Disk](https://cstack.github.io/db_tutorial/parts/part5.html)
- 230924: [Our First Tests (and Bugs)](https://cstack.github.io/db_tutorial/parts/part4.html)
- 230923: [An In-Memory, Append-Only, Single-Table Database](https://cstack.github.io/db_tutorial/parts/part3.html)
- 230921: [World’s Simplest SQL Compiler and Virtual Machine](https://cstack.github.io/db_tutorial/parts/part2.html)
- 230920: [Introduction and Setting up the REPL](https://cstack.github.io/db_tutorial/parts/part1.html)
