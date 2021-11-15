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
daddi R4, R4, 8
dmtc1 R4, F12
daddi R4, R4, 8
dmtc1 R4, F18
daddi R4, R4, 8
dmtc1 R4, F20

sdc1 F8, 0(R1)

cvt.d.l F6, F6
mul.d F8, F2, F6
cvt.d.l F12, F12
mul.d F16, F2, F12
cvt.d.l F18, F18
mul.d F22, F2, F18
cvt.d.l F20, F20

mul.d F24, F2, F20
daddi R1, R1, 32
mul.d F8, F8, F8

mul.d F28, F4, F20
mul.d F16, F16, F16
mul.d F22, F22, F22
mul.d F24, F24, F24
mul.d F10, F4, F6

sdc1 F16, -24(R1)

mul.d F14, F4, F12
sdc1 F22, -16(R1)

mul.d F26, F4, F18
sdc1 F24, -8(R1)

sdc1 F10, 0(R2)
sdc1 F14, 8(R2)
sdc1 F26, 16(R2)
sdc1 F28, 24(R2)

daddi R2, R2, 32
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
