
a = (0, 0x48656c6c6f2021)
b = (0, 65537)
private_expo = (0x00cab10ccaa4437b67, 0x11c977a277fe00a1)
c = (0xe037d35a8b160eb7, 0xf11919bfef440917)

a = a[0] << 64 | a[1]
b = b[0] << 64 | b[1]
private_expo = private_expo[0] << 64 | private_expo[1]
c = c[0] << 64 | c[1]

out = pow(a, b, c)

first_64 = out >> 64
second_64 = out & ((1 << 64) - 1)
print(hex(first_64) + " " + hex(second_64))

out = pow(out, private_expo, c)

first_64 = out >> 64
second_64 = out & ((1 << 64) - 1)
print(hex(first_64) + " " + hex(second_64))
