.data
# char * data[] = {"Joe", "Jenny", "Jill", "John", "Jeff", "Joyce", "Jerry", "Janice", "Jake", "Jonna", "Jack", "Jocelyn", "Jessie", "Jess", "Janet", "Jane"};
            .align 5  # consider names with a 32-byte boundary
dataName:   .asciiz "Joe"
            .align 5
            .asciiz "Jenny"
            .align 5
            .asciiz "Jill"
            .align 5
            .asciiz "John"
            .align 5
            .asciiz "Jeff"
            .align 5
            .asciiz "Joyce"
            .align 5
            .asciiz "Jerry"
            .align 5
            .asciiz "Janice"
            .align 5
            .asciiz "Jake"
            .align 5
            .asciiz "Jonna"
            .align 5
            .asciiz "Jack"
            .align 5
            .asciiz "Jocelyn"
            .align 5
            .asciiz "Jessie"
            .align 5
            .asciiz "Jess"
            .align 5
            .asciiz "Janet"
            .align 5
            .asciiz "Jane"

            .align 2  # addresses should start on a word boundary
dataAddr:   .space 64 # 16 pointers to strings: 16*4 = 64

# int size = 16;
size:       .word 16

PRINT1:     .asciiz "Initial array is:\n"
PRINT2:     .asciiz "Insertion sort is finished!\n"

# int size = 16;
# size:       16


.text
.globl main


# int main(void)
main:
    subu $sp,$sp,32
    sw $ra,20($sp)


# printf("Initial array is:\n");
    li $v0, 4
    la $a0, PRINT1
    syscall

# print_array(data, size);
    la $a0, dataName    # load address of array
    la $a1, size
    jal print_array


#   insertSort(data, size);
    la $a0, dataName    # load address of array
    la $a1, size
    jal insertSort


#   printf("Insertion sort is finished!\n");
    li $v0, 4
    la $a0, PRINT2
    syscall

# print_array(data, size);
    la $a0, dataName    # load address of array
    la $a1, size
    jal print_array

# exit(0);
    li $v0, 10
    syscall




# ############ FUNCTION #############
# void insertSort(char * a[], size_t length);
insertSort:
    subu $sp,$sp,32
    sw $ra,20($sp)


    jr $ra # Return to caller



# ############ FUNCTION #############
# int str_lt (char *x, char *y)
str_lt:
    subu $sp,$sp,32
    sw $ra,20($sp)


    jr $ra # Return to caller


# ############ FUNCTION #############
# void print_array(char * a[], const int size)
.data
LBRKT:  .asciiz "["
RBRKT:  .asciiz "]\n"
SPACES:   .asciiz "  "
NEWLINE:.asciiz "\n"

.text
print_array:
    subu $sp,$sp,32
    sw $ra,20($sp)
    # sw $a0,0($sp) # Save argument (size):  0($sp) is the variable 'size'
    # lw $t0,0($sp) # Load size ($t0 is size)
    
    move $t0, $a0   # Load Array Address into t0. ($t0 is array's first element address)
    lw $t1, 0($a1) # Load Argument Size into t0. ($t0 is size)
    
#  int i=0;
    li $t2, 0

# printf("[");
    li $v0, 4
    la $a0, LBRKT
    syscall


# while(i < size)
loop:

# printf("  %s", a[i++]);
    li $v0, 4
    la $a0, SPACES
    syscall

    li $v0, 4
    move $a0, $t0
    syscall

# i++    
    add $t2, $t2, 1
    add $t0, $t0, 32
    blt $t2, $t1, loop # Branch on less than. i < size

# printf(" ]\n");
    li $v0, 4
    la $a0, RBRKT
    syscall

    jr $ra # Return to caller
