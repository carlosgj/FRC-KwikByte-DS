filename = "C:\\Users\\carlosj\\Documents\\DS\\unk.bin"
fob = open(filename, 'rb')
data = fob.read()
fob.close()
for i, char in enumerate(data):
    if char == '\x53':
        if data[i-1] == '\xef':
            print i-1
