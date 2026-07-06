import math

rings = 10
segments = 20
radius = 1.5

vertices = []
uvs = []
normals = []
faces = []

for i in range(rings + 1):
    theta = math.pi * i / rings
    y = math.cos(theta) * radius
    r = math.sin(theta) * radius

    for j in range(segments):
        phi = 2 * math.pi * j / segments
        x = math.cos(phi) * r
        z = math.sin(phi) * r

        vertices.append((x, y, z))

        u = j / segments
        v = i / rings
        uvs.append((u, v))

        length = math.sqrt(x * x + y * y + z * z)
        normals.append((x / length, y / length, z / length))

for i in range(rings):
    for j in range(segments):
        a = i * segments + j + 1
        b = i * segments + ((j + 1) % segments) + 1
        c = (i + 1) * segments + ((j + 1) % segments) + 1
        d = (i + 1) * segments + j + 1

        if i != 0:
            faces.append((a, b, d))

        if i != rings - 1:
            faces.append((b, c, d))

with open("res/models/earth_ocean.obj", "w") as file:
    file.write("# Low poly ocean sphere with UVs\n")
    file.write("o earth_ocean\n")

    for x, y, z in vertices:
        file.write(f"v {x:.6f} {y:.6f} {z:.6f}\n")

    for u, v in uvs:
        file.write(f"vt {u:.6f} {v:.6f}\n")

    for x, y, z in normals:
        file.write(f"vn {x:.6f} {y:.6f} {z:.6f}\n")

    file.write("s off\n")

    for face in faces:
        file.write("f " + " ".join(f"{index}/{index}/{index}" for index in face) + "\n")

print("Created res/models/earth_ocean.obj")