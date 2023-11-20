# 读取文件内容并打印
def read_file(path):
    with open(path, 'r') as f:
        print(f.read())

# 写入文件
def write_file(path, msg):
    with open(path, 'w') as f:
        f.write(msg)
