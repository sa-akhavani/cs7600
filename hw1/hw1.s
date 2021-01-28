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
# size:       16


.text
.globl main


main:
    subu $sp,$sp,32
    sw $ra,20($sp)

# int size = 16;
    la $a0, dataName    # load address of array
    li $a1, 16

    jal print_array
    li $v0, 10
    syscall




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
    add $t1, $a1, 0 # Load Argument Size into t0. ($t0 is size)
    
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
