import subprocess
import sys
import argparse

def erase_flash(com_port):
    """Erase flash region at 0x390000  or 0xf90000"""
    python_path = r"C:\Users\embeddedmachan\.espressif\python_env\idf5.2_py3.11_env\Scripts\python.exe"
    esptool_path = r"C:\Users\embeddedmachan\esp\v5.2.2\esp-idf\components\esptool_py\esptool\esptool.py"
    
    command = [
        python_path,
        esptool_path,
        "-p", com_port,
         "erase_region", "0xf90000", "0x10000"  # Added size parameter (adjust as needed)# "erase_region", "0x390000", "0x10000"  # Added size parameter (adjust as needed)
    ]
    
    print(f"Executing erase command: {' '.join(command)}")
    try:
        subprocess.run(command, check=True)
        print("Erase operation completed successfully.")
    except subprocess.CalledProcessError as e:
        print(f"Error during erase operation: {e}")
        sys.exit(1)

def upload_filesystem(com_port):
    """Upload filesystem to flash at 0x390000"""
    python_path = r"C:\Users\embeddedmachan\.espressif\python_env\idf5.2_py3.11_env\Scripts\python.exe"
    esptool_path = r"C:\Users\embeddedmachan\esp\v5.2.2\esp-idf\components\esptool_py\esptool\esptool.py"
    fs_path = r".\disk.fs"
    
    command = [
        python_path,
        esptool_path,
        "-p", com_port,
        "write_flash", "0xf90000", fs_path
    ]
    
    print(f"Executing upload command: {' '.join(command)}")
    try:
        subprocess.run(command, check=True)
        print("Filesystem upload completed successfully.")
    except subprocess.CalledProcessError as e:
        print(f"Error during filesystem upload: {e}")
        sys.exit(1)

def update_lua_files():
    """Update Lua files using luadb.py"""
    luadb_path = r".\luadb.py"
    demo_path = r"..\demo"
    
    command = [
        "python",
        luadb_path,
        demo_path
    ]
    
    print(f"Executing Lua update command: {' '.join(command)}")
    try:
        subprocess.run(command, check=True)
        print("Lua files update completed successfully.")
    except subprocess.CalledProcessError as e:
        print(f"Error during Lua files update: {e}")
        sys.exit(1)

def main():
    parser = argparse.ArgumentParser(description="ESP32 Flash Management Tool")
    subparsers = parser.add_subparsers(dest='command', help='Available commands')
    
    # Erase command
    erase_parser = subparsers.add_parser('erase', help='Erase flash region')
    erase_parser.add_argument('com_port', help='COM port (e.g., COM3)')
    
    # Upload command
    upload_parser = subparsers.add_parser('upload', help='Upload filesystem')
    upload_parser.add_argument('com_port', help='COM port (e.g., COM3)')
    
    # Update command
    update_parser = subparsers.add_parser('update', help='Update Lua files')
    
    args = parser.parse_args()
    
    if args.command == 'erase':
        erase_flash(args.com_port)
    elif args.command == 'upload':
        upload_filesystem(args.com_port)
    elif args.command == 'update':
        update_lua_files()
    else:
        parser.print_help()
        sys.exit(1)

if __name__ == "__main__":
    main()