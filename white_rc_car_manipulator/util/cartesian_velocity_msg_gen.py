import struct

def generate_hex_string(vx, vy, w):
    vx_bytes = struct.pack('f', vx)
    vy_bytes = struct.pack('f', vy)
    w_bytes = struct.pack('f', w)
    return ' '.join(format(byte, '02X') for byte in vx_bytes + vy_bytes + w_bytes)

def HIBYTE(w):
    return (w >> 8) & 0xFF

def LOBYTE(w):
    return w & 0xFF

def main():
    vx = -10.0 # float(input("Enter vx: "))
    vy = 0.0 # float(input("Enter vy: "))
    w = 0.0 # float(input("Enter w: "))

    data_hex_str = generate_hex_string(vx, vy, w)
    len_data = len(data_hex_str.split())
    cmd_id = 0x0100
    print(f"len_data: {len_data}")
    print(f"prefix: 5A F0 00 00 00 {LOBYTE(len_data):02X} {HIBYTE(len_data):02X} 00 00 {LOBYTE(cmd_id):02X} {HIBYTE(cmd_id):02X}")
    print(f"data: {data_hex_str}")

if __name__ == "__main__":
    main()
