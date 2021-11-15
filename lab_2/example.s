 
.data
v: .space 16

.text
addi $1, $0, 4  
addi $2, $0, 1
addi $3, $0, 0

loop:
dmult $2, $1
mflo $2
sw $2, v($3)
addi $3, $3, 4
addi $1, $1, -1
bne $1, $0, loop
syscall 0
