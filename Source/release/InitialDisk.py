import os

# 创建文件夹
os.makedirs("Disk", exist_ok=True)

# 创建一百个文件并写入空格
for i in range(100):
    filename = f"Disk/block{i}.txt"
    with open(filename, "w") as file:
        file.write(" " * 10000)
