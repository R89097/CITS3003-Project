import math
import re

input_path = "res/models/earth.obj"
output_path = "res/models/earth_fixed.obj"

vertices = []
lines = []

with open(input_path, "r") as file:
    lines = file.readlines()

# 先读取所有 v 顶点
for line in lines:
    if line.startswith("v "):
        parts = line.strip().split()
        x = float(parts[1])
        y = float(parts[2])
        z = float(parts[3])
        vertices.append((x, y, z))

vt_lines = []

# 为每个 vertex 生成一个 vt
for x, y, z in vertices:
    radius = math.sqrt(x * x + y * y + z * z)

    if radius == 0:
        u = 0.5
        v = 0.5
    else:
        u = 0.5 + math.atan2(z, x) / (2 * math.pi)
        v = 0.5 - math.asin(y / radius) / math.pi

    vt_lines.append(f"vt {u:.6f} {v:.6f}\n")

output_lines = []
vt_inserted = False

for line in lines:
    # 在第一个 vn 前插入所有 vt
    if line.startswith("vn ") and not vt_inserted:
        output_lines.extend(vt_lines)
        vt_inserted = True

    # 把 f 44//1 改成 f 44/44/1
    if line.startswith("f "):
        parts = line.strip().split()
        new_parts = ["f"]

        for item in parts[1:]:
            match = re.match(r"(\d+)//(\d+)", item)
            if match:
                vertex_index = match.group(1)
                normal_index = match.group(2)
                new_parts.append(f"{vertex_index}/{vertex_index}/{normal_index}")
            else:
                new_parts.append(item)

        output_lines.append(" ".join(new_parts) + "\n")
    else:
        output_lines.append(line)

with open(output_path, "w") as file:
    file.writelines(output_lines)

print("Done! Created:", output_path)