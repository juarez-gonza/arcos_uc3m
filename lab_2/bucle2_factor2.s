.data
v: .space 256
w: .space 256
x: .double 2.0
y: .double 1.5
z: .double 0.0
.text
daddi R1, R0, v
daddi R2, R0, w
daddi R3, R0, 256

ldc1 F2, x(R0)
ldc1 F4, y(R0)
daddi R4, R0, 0

loop1:
dmtc1 R4, F6
cvt.d.l F6, F6
mul.d F8, F2, F6
mul.d F8, F8, F8
sdc1 F8, 0(R1)
mul.d F10, F4, F6
sdc1 F10, 0(R2)
daddi R1, R1, 8
daddi R2, R2, 8
daddi R4, R4, 8
bne R4, R3, loop1

daddi R1, R0, v
daddi R2, R0, w
daddi R4, R0, 0
ldc1 F8, z(R0)


loop2:
ldc1 F2, 0(R1)
ldc1 F10, 8(R1)
mul.d F6, F2, F4
mul.d F14, F10, F12
daddi R1, R1, 16
ldc1 F4, 0(R2)
ldc1 F12, 8(R2)
daddi R2, R2, 16
daddi R4, R4, 16
add.d F8, F8, F6
add.d F16, F16, F14
bne R4, R3, loop2

sdc1 F8, z(R0)
syscall 0