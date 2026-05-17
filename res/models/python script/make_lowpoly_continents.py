import math
import random

outer_radius = 1.56
inner_radius = 1.42

continents = [
    (25, 35, 0.85, 0.55),
    (-15, 145, 1.95, 0.60),
    (20, 260, 1.10, 0.65),
]

vertices = []
uvs = []
normals = []
faces = []

def sphere_point(lat_deg, lon_deg, r):
    lat = math.radians(lat_deg)
    lon = math.radians(lon_deg)
    x = r * math.cos(lat) * math.cos(lon)
    y = r * math.sin(lat)
    z = r * math.cos(lat) * math.sin(lon)
    return (x, y, z)

def normalize(p):
    x, y, z = p
    length = math.sqrt(x*x + y*y + z*z)
    return (x/length, y/length, z/length)

for lat, lon, width, height in continents:
    points = 16

    top_center_index = len(vertices) + 1
    top_center = sphere_point(lat, lon, outer_radius)
    vertices.append(top_center)
    uvs.append((0.5, 0.5))
    normals.append(normalize(top_center))

    bottom_center_index = len(vertices) + 1
    bottom_center = sphere_point(lat, lon, inner_radius)
    vertices.append(bottom_center)
    uvs.append((0.5, 0.5))
    normals.append(normalize(bottom_center))

    top_ring = []
    bottom_ring = []

    random.seed(int(lat * 1000 + lon))

    offsets = []
    for i in range(points):
        angle = 2 * math.pi * i / points

        offset_lat = math.sin(angle) * height * random.uniform(0.75, 1.25)
        offset_lon = math.cos(angle) * width * random.uniform(0.75, 1.25)

        offsets.append((offset_lat, offset_lon))

    for i, (offset_lat, offset_lon) in enumerate(offsets):
        p = sphere_point(lat + offset_lat * 20, lon + offset_lon * 20, outer_radius)
        vertices.append(p)
        uvs.append((i / points, 1.0))
        normals.append(normalize(p))
        top_ring.append(len(vertices))

    for i, (offset_lat, offset_lon) in enumerate(offsets):
        p = sphere_point(lat + offset_lat * 20, lon + offset_lon * 20, inner_radius)
        vertices.append(p)
        uvs.append((i / points, 0.0))
        normals.append(normalize(p))
        bottom_ring.append(len(vertices))

    # top
    for i in range(points):
        a = top_center_index
        b = top_ring[i]
        c = top_ring[(i + 1) % points]
        faces.append((a, c, b))

    # side
    for i in range(points):
        top_a = top_ring[i]
        top_b = top_ring[(i + 1) % points]
        bottom_a = bottom_ring[i]
        bottom_b = bottom_ring[(i + 1) % points]

        faces.append((top_a, top_b, bottom_b))
        faces.append((top_a, bottom_b, bottom_a))

    # bottom
    for i in range(points):
        a = bottom_center_index
        b = bottom_ring[(i + 1) % points]
        c = bottom_ring[i]
        faces.append((a, c, b))

with open("res/models/lowpoly_continents.obj", "w") as file:
    file.write("# Three thick low poly continents\n")
    file.write("o lowpoly_continents\n")

    for x, y, z in vertices:
        file.write(f"v {x:.6f} {y:.6f} {z:.6f}\n")

    for u, v in uvs:
        file.write(f"vt {u:.6f} {v:.6f}\n")

    for x, y, z in normals:
        file.write(f"vn {x:.6f} {y:.6f} {z:.6f}\n")

    file.write("s off\n")

    for face in faces:
        file.write("f " + " ".join(f"{i}/{i}/{i}" for i in face) + "\n")

print("Created res/models/lowpoly_continents.obj")