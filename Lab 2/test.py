#{8, 5} = 69
#{1, 2} = 10

for a_ in range(1, 256):
    for a2_ in range(0, 256):
        combined = (a_ << 8) + a2_

        for b_ in range(1, 256):
            for b2_ in range(1, 256):
                combined2 = (b_ << 8) + b2_

                mod = combined % combined2

                c = mod >> 8
                c2 = mod & 0xFF

                a = a_
                a2 = a2_
                b = b_
                b2 = b2_

                if b == 0:
                    guess_hi = 0
                    guess_lo = a2 % b2
                elif a == 0:
                    guess_hi = 0
                    guess_lo = a2 % b2
                else:
                    while (a > b) or (a == b  and a2 >= b2):
                        a -= b
                        if a2 < b2:
                            a -= 1
                            a2_copy = a2
                            a2 = 0xFF - b2
                            a2 += a2_copy + 1
                        else:
                            a2 -= b2                            

                    guess_hi = a
                    guess_lo = a2

                if guess_lo != c2:
                    print("Failed: %d %d | %d %d | %d %d" % (a, a2, b, b2, c, c2))
                    exit(1)