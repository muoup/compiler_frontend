# Rich Enums and Structs
Idea: Support for Rust-like enums, and non-class-like structs
Additionally, any syntactic additions should not override existing C syntax, and should be as unobtrusive as possible.

## Enums

### Current Enum Syntax
In C, there are main ways enums are implemented, and they are as follows:

1. **Indexed Enums**: Simple named integer constants, allow some to be set to specific values.
    ```c
    enum Color {
        RED, // RED = 0
        GREEN, // GREEN = 1
        BLUE = 5 // BLUE = 5
    };
    ```

2. **Bitwise Enums**: Allow for bitwise operations on enums.
    ```c
    enum Permissions {
        READ = 0b001, // READ = 1
        WRITE = 0b010, // WRITE = 2
        EXECUTE = 0b100 // EXECUTE = 4
    };
    ```

### New Enum Syntax

Simplified bitwise enums, with implicit values.
```c
bitwise enum Permissions {
    READ, // Implicitly set to 0b001
    WRITE, // Implicitly set to 0b010
    EXECUTE // Implicitly set to 0b100
};

// It seems more intuitive to borrow from C++ bitset syntax and allow for [] indexing
bool can_write(Permissions perms) {
    return perms[WRITE];
}

void set_write(Permissions* perms) {
    perms[WRITE] = true;
}
```

Union-type enums, a.k.a. rust-like rich enums:
```c
struct UnionVTableEntry;

enum Identifier {
    ID(int),
    Name(char[32])
};

enum EnumConstant {
    Index {
        char[32] name;
        int value;
    },
    Bitwise {
        char[32] name;
    },
    Union {
        char[32] name;
        void* data;
        UnionVTableEntry* vtable;
    }
};

enum MixedEnum {
    Constant = 1, // Implicity of type i32
    Type(u64),
    Struct {
        char[32] name;
        int value;
    }
};
```

## Structs

### Current Struct Syntax

In C, structs are simple well-ordered structures of data, and can be defined like such:
```c
struct Point {
    int x;
    int y;
};
```

In C++, structs can have methods attached as a syntax sugar like so. This approach is fine, but
often leads to too much abstraction for many people's taste.
```c++
struct Point {
    int x;
    int y;

    void print() {
        printf("(%d, %d)\n", x, y);
    }
};
```

Versus the C equivalent:
```c
struct Point {
    int x;
    int y;
};

void Point_print(struct Point* self) {
    printf("(%d, %d)\n", self->x, self->y);
}
```

### New Struct Syntax

#### Idea: Composite constructors

Instead of constructors, which may also be supported separately, a struct can be initialized with a system
similar to make rules. For every rule starting with a '.', if all left-side field names are provided,
the fields on the right side of the arrow are set to the appropriate value. This essentially allows for
a multi-set initialization process which can be validated at compile time. For cases where rules may overlap, 
whichever rule is defined first will be executed first, and any fields which may already be set from previous 
rules will be ignored.

The main drawback to this approach is that it may be difficult to determine what set of parameters will satisfy
enough rules to initialize the struct. Realistically the solution to this is choice, i.e. if the developer decides
a rules-based initialization would be too convoluted, they may opt for traditional C-style initialization,
or C++-style constructors.

```c  
extern u64 get_book_id(char* book_name);
extern u64 get_store_id(char* store_name);
 
struct Book {
   u64 nameID;
   u64 storeID;
   
   // Implicit: .u64 nameID => nameID; .u64 storeID => storeID;
   // If not desired, ^nameID and ^storeID can be used to prevent implicit direct initialization
   
   .char* bookName => nameID(get_book_id(name));
   .char* storeName, u64 zipCode => storeID(get_store_id(store));
};
 
int main() {
   // Preserves C-style initializer list
   struct Book book {
      .name = 1234567890,
      
      .storeName = "Barnes & Noble",
      .zipCode = 12345
   };
   
   book.bookName = "The Catcher in the Rye";
   
   struct Book incorrectly_initialized {
      .name = "The Great Gatsby",
      
      .zipCode = 12345
   }; // Error: no known rule found to initialize storeID
}
```

### Equivalent Code:

There are a few different ways to recreate the equivalent functionality of the above code in C or C++, however
they all have their own issues which this should aim to solve.

1. C++ constructors: Simple and clean, however the amount of constructors required can quickly get out of hand.

   ```c++
   struct Book {
       uint64_t nameID;
       uint64_t storeID;
       
       Book(uint64_t nameID, uint64_t storeID) : nameID(nameID), storeID(storeID) {}
       Book(const char* name, uint64_t storeID) : nameID(get_book_id(name)), storeID(storeID) {}
       Book(uint64_t nameID, const char* store) : nameID(nameID), storeID(get_store_id(store)) {}
       Book(const char* name, const char* store) : nameID(get_book_id(name)), storeID(get_store_id(store)) {}
   };

   int main() {
       Book book { "The Great Gatsby", 1234567890 };
   }
   ```

2. C++ template + visitor pattern: Not clean, however does not require as many constructors. This looks horrible I'm not sure anyone would use this.

   ```c++
   struct BookFix1 {
       uint64_t nameID;
       uint64_t storeID;
       
       uint64_t v_nameID(char* name) {
           return get_book_id(name);
       }
       
       uint64_t v_storeID(char* store) {
           return get_store_id(store);
       }
       
       uint64_t v_nameID(uint64_t id) {
           return id;
       }
       
       uint64_t v_storeID(uint64_t id) {
           return id;
       }
       
       template<typename NameType, typename StoreType>
       BookFix1(NameType name, StoreType store) : nameID(v_nameID(name)), storeID(v_storeID(store)) {}
   };
   ```

3. C-style non-class-like structs: Very clean, however lacks any way to validate the struct has been fully initialized. This is often very useful for preventing errors.

   ```c
   extern u64 get_book_id(char* book_name);
   extern u64 get_store_id(char* store_name);

   struct BookFix2 {
       uint64_t nameID;
       uint64_t storeID;
   };

   int main() {
       struct BookFix2 book {
           .nameID = get_book_id("The Great Gatsby"),
           .storeID = 1234567890
       };
   }
   ```

4. C++ Struct Datatypes: This is a bit hacky but is probably the most equivalent. It's very unintuitive and feels unidiomatic, however it does work.

   ```c++
   extern u64 get_book_id(char* book_name);
   extern u64 get_store_id(char* store_name);

   struct Book {
       struct BookName {
           u64 id;
           
           BookName(char* name) : id(get_book_id(name)) {}
           BookName(u64 id) : id(id) {}
       } data;
       
       struct StoreName {
           u64 id;
           
           StoreName(char* name) : id(get_store_id(name)) {}
           StoreName(u64 id) : id(id) {}
       } store;
   };

   int main() {
       struct Book book {
           .data = "The Great Gatsby",
           .store = 1234567890
       };
   }
   ```