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

.text
.globl main

# int main(void)
main:
    subu $sp,$sp,32
    sw $ra,20($sp)

# Fill dataAddr with addresses of dataNames
    la $t0, dataName
    la $t1, size
    lw $t1, 0($t1)
    la $t3, dataAddr
    li $t4, 0
loop_1:
    move $t2, $t0 # move address of dataName[i] to dataAddr[i]
    sw $t2, 0($t3)

    add $t4, $t4, 1 # i++
    add $t0, $t0, 32 # go to next element of dataName
    add $t3, $t3, 4 # go to next element of dataAddr
    blt $t4, $t1, loop_1 # Branch on less than. i < size


# printf("Initial array is:\n");
    li $v0, 4
    la $a0, PRINT1
    syscall

# print_array(data, size);
    la $a0, dataAddr    # load address of array
    la $a1, size
    jal print_array


#   insertSort(data, size);
    la $a0, dataAddr    # load address of array
    la $a1, size
    jal insertSort


#   printf("Insertion sort is finished!\n");
    li $v0, 4
    la $a0, PRINT2
    syscall

# print_array(data, size);
    la $a0, dataAddr    # load address of array
    la $a1, size
    jal print_array


    lw $ra,20($sp) # Restore return address
    addiu $sp,$sp,32 # Pop stack frame

# exit(0);
    li $v0, 10
    syscall




# ############ FUNCTION #############
# void insertSort(char * a[], size_t length);
insertSort:
    subu $sp,$sp,32
    sw $ra,20($sp)

# i = 1
    li $t0, 1   # i is $t0
    move $t1, $a0   # Load Array Address into t1. ($t1 is array's first element address)
    lw $t2, 0($a1) # Load Argument Size into t2. ($t2 is length)
#    add $t1, $t1, 4 # t1 is array's second element


# for(i = 1; i < length; i++)
loop_2:
# char *value = a[i];
    mul $t3, $t0, 4
    add $t3, $t1, $t3
    lw $t3, 0($t3)  # t3 is value = a[i]


# j = i-1
    sub $t4, $t0, 1 # t4 is j
# for (j = i-1; j >= 0 && str_lt(value, a[j]); j--)
loop_3:
# a[j]    
    mul $t5, $t4, 4
    add $t5, $t1, $t5
#    lw $t5, 0($t5)  # t5 is a[j]
    move $a1, $t5
# value
    move $a0, $t3

    jal str_lt
    move $t6, $v0 # t6 is the return value of str_lt

# j--
    sub $t4, $t4, 1 # t4 is j

# condition str_lt(value, a[j])
    blez $t6, exit_loop_3
# j >= 0
    bgez $t4, loop_3

exit_loop_3:
# a[j+1] = value;
    add $t4, $t4, 1 # j+1
    mul $t6, $t4, 4
    add $t6, $t1, $t6
    sw $t3, 0($t6)  # t6 is a[j]. Store value into a[j+1]

# i++    
    add $t0, $t0, 1
    blt $t0, $t2, loop_2 # Branch on less than. i < size

    lw $ra,20($sp) # Restore return address
    addiu $sp,$sp,32 # Pop stack frame
    jr $ra # Return to caller



# ############ FUNCTION #############
.data
    ENDCAR:  .ascii "\0"
.text
# int str_lt (char *x, char *y)
str_lt:
    subu $sp,$sp,32
    sw $ra,20($sp)

    move $t0, $a0   # Address of char* x
    move $t1, $a1 # Address of char* y
    la $t7, ENDCAR
    lw $t7, 0($t7) # load "\0" into t7

#  for (; *x!='\0' && *y!='\0'; x++, y++)
loop_str:
    lb $t2, 0($t0)
    lb $t3, 0($t1)
# *x!='\0'
    sub $t5, $t2, $t7
    beqz $t5, after_loop

# *y!='\0'
    sub $t5, $t3, $t7
    beqz $t5, after_loop

#    if ( *x < *y ) return 1;
    sub $t4, $t2, $t3
    bltz $t4, end_fn_with_1

#    if ( *y < *x ) return 0;
    sub $t4, $t3, $t2
    bltz $t4, end_fn_with_0

    add $t0, $t0, 1
    add $t1, $t1, 1
    b loop_str
    
#  if ( *y == '\0' ) return 0;    
after_loop:
    sub $t5, $t3, $t7
    beqz $t5, end_fn_with_0

#  else return 1;
# return 1
end_fn_with_1:
    li $v0, 1 # return value 1 of function
    b return_fn

# return 0
end_fn_with_0:
    li $v0, 0 # return value 0 of function

return_fn:
    lw $ra,20($sp) # Restore return address
    addiu $sp,$sp,32 # Pop stack frame
    jr $ra # Return to caller


# ############ FUNCTION #############
# void print_array(char * a[], const int size)
.data
LBRKT:  .asciiz "["
RBRKT:  .asciiz "]\n"
SPACES: .asciiz "  "
NEWLINE:.asciiz "\n"

.text
print_array:
    subu $sp,$sp,32
    sw $ra,20($sp)
    
    move $t0, $a0   # Load Array Address into t0. ($t0 is array's first element address)
    lw $t1, 0($a1) # Load Argument Size into t1. ($t1 is size)
    
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
    lw $a0, 0($t0)
    syscall

# i++    
    add $t2, $t2, 1
    add $t0, $t0, 4
    blt $t2, $t1, loop # Branch on less than. i < size

# printf(" ]\n");
    li $v0, 4
    la $a0, RBRKT
    syscall

    lw $ra,20($sp) # Restore return address
    addiu $sp,$sp,32 # Pop stack frame
    jr $ra # Return to caller
