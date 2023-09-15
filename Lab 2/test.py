
a = (0 << 128) | 321
b = (2 << 128) | 5
c = (0 << 128) | 0xffffffffff

out = pow(a, b, c)

first_128 = out >> 128
second_128 = out & ((1 << 128) - 1)
print('ciphertext=' + str(first_128) + " " + str(second_128))