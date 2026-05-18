import math
import random

cloud_radius = 1.90

continents = [
    (25, 35),
    (-15, 145),
    (20, 260),
]

clouds_per_continent = [4, 5, 5]

vertices = []
uvs = []
normals = []
faces = []

def normalize(p):
    x, y, z = p
    length = math.sqrt(x * x + y * y + z * z)
    return (x / length, y / length, z / length)

def sphere_point(lat_deg, lon_deg, radius):
    lat = math.radians(lat_deg)
    lon = math.radians(lon_deg)

    x = radius * math.cos(lat) * math.cos(lon)
    y = radius * math.sin(lat)
    z = radius * math.cos(lat) * math.sin(lon)

    return (x, y, z)

def cross(a, b):
    return (
        a[1] * b[2] - a[2] * b[1],
        a[2] * b[0] - a[0] * b[2],
        a[0] * b[1] - a[1] * b[0],
    )

def add_lowpoly_sphere(center, radius, rings=5, segments=8):
    cx, cy, cz = center
    start = len(vertices) + 1

    for i in range(rings + 1):
        theta = math.pi * i / rings
        y = math.cos(theta) * radius
        ring_radius = math.sin(theta) * radius

        for j in range(segments):
            phi = 2 * math.pi * j / segments
            x = math.cos(phi) * ring_radius
            z = math.sin(phi) * ring_radius

            vertices.append((cx + x, cy + y, cz + z))

            u = j / segments
            v = i / rings
            uvs.append((u, v))

            normals.append(normalize((x, y, z)))

    for i in range(rings):
        for j in range(segments):
            a = start + i * segments + j
            b = start + i * segments + ((j + 1) % segments)
            c = start + (i + 1) * segments + ((j + 1) % segments)
            d = start + (i + 1) * segments + j

            if i != 0:
                faces.append((a, d, b))

            if i != rings - 1:
                faces.append((b, d, c))

def add_cloud(lat, lon, seed):
    random.seed(seed)

    direction = normalize(sphere_point(lat, lon, 1.0))
    center = sphere_point(lat, lon, cloud_radius)

    up = (0, 1, 0)
    if abs(direction[1]) > 0.9:
        up = (1, 0, 0)

    tangent1 = normalize(cross(up, direction))
    tangent2 = normalize(cross(direction, tangent1))

    pieces = []

    # 一个大球
    pieces.append((0.00, 0.00, 0.00, random.uniform(0.18, 0.24)))

    # 两三个中球
    medium_count = random.randint(2, 3)
    for _ in range(medium_count):
        pieces.append((
            random.uniform(-0.20, 0.22),
            random.uniform(-0.08, 0.10),
            random.uniform(-0.02, 0.04),
            random.uniform(0.11, 0.16),
        ))

    # 几个小球，放在两边，让它像云团
    small_count = random.randint(3, 5)
    for _ in range(small_count):
        side = random.choice([-1, 1])
        pieces.append((
            side * random.uniform(0.22, 0.38),
            random.uniform(-0.08, 0.08),
            random.uniform(-0.03, 0.03),
            random.uniform(0.055, 0.095),
        ))

    for offset1, offset2, height, size in pieces:
        px = center[0] + tangent1[0] * offset1 + tangent2[0] * offset2 + direction[0] * height
        py = center[1] + tangent1[1] * offset1 + tangent2[1] * offset2 + direction[1] * height
        pz = center[2] + tangent1[2] * offset1 + tangent2[2] * offset2 + direction[2] * height

        add_lowpoly_sphere((px, py, pz), size, rings=5, segments=8)

random.seed(3003)

for continent_index, (lat, lon) in enumerate(continents):
    count = clouds_per_continent[continent_index]

    for i in range(count):
        angle = 2 * math.pi * i / count

        lat_offset = math.sin(angle) * random.uniform(8, 15)
        lon_offset = math.cos(angle) * random.uniform(12, 22)

        add_cloud(
            lat + lat_offset,
            lon + lon_offset,
            3003 + continent_index * 100 + i
        )

with open("res/models/lowpoly_clouds.obj", "w") as file:
    file.write("# Low poly fluffy cloud groups\n")
    file.write("o lowpoly_clouds\n")

    for x, y, z in vertices:
        file.write(f"v {x:.6f} {y:.6f} {z:.6f}\n")

    for u, v in uvs:
        file.write(f"vt {u:.6f} {v:.6f}\n")

    for x, y, z in normals:
        file.write(f"vn {x:.6f} {y:.6f} {z:.6f}\n")

    file.write("s off\n")

    for face in faces:
        file.write("f " + " ".join(f"{i}/{i}/{i}" for i in face) + "\n")

print("Created res/models/lowpoly_clouds.obj")