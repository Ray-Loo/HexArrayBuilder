#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <iomanip>

using namespace std;

#define Dbg(fmt, ...) printf(fmt, ##__VA_ARGS__)

void VisualizeByte(unsigned char b, size_t index)
{
    // 可视化：每处理一个字节，就打印出来
    printf("[Index %08zu] 0x%02X\n", index, b);
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        Dbg("Usage: %s source_file dest_file\n", argv[0]);
        return 1;
    }

    try {
        // 打开源文件
        ifstream source_file(argv[1], ios::binary | ios::ate);
        if (!source_file.is_open()) {
            cout << "Error opening source file: " << argv[1] << endl;
            return 1;
        }

        // 获取文件大小
        streamsize file_size = source_file.tellg();
        source_file.seekg(0, ios::beg);

        vector<unsigned char> buffer(file_size);

        // 读取文件
        if (!source_file.read(reinterpret_cast<char*>(buffer.data()), file_size)) {
            cout << "Error reading file: " << argv[1] << endl;
            return 1;
        }
        source_file.close();


        //==============================
        // 生成 C 语言风格的数组头部
        //==============================
        ofstream dest_file(argv[2], ios::binary);
        if (!dest_file.is_open()) {
            cout << "Error opening dest file: " << argv[2] << endl;
            return 1;
        }
        // 写入头部
        dest_file << "unsigned char peData[] = {\n";

        //==============================
        // 实时可视化并输出数组内容
        //==============================
        for (size_t i = 0; i < buffer.size(); ++i)
        {
            unsigned char b = buffer[i];

            // 实时可视化数据流
            VisualizeByte(b, i);

            // 写入目标文件
            dest_file << "0x" << hex << uppercase << setw(2) << setfill('0') << (int)b;

            if (i != buffer.size() - 1)
                dest_file << ", ";

            if ((i + 1) % 16 == 0)
                dest_file << "\n";// 每 16 个字节换行
        }

        //==============================
        // 写入尾部
        //==============================
        dest_file << "\n};\n";
        dest_file.close();

        cout << "\nConvert Success! File size: " << file_size << endl;

    }
    catch (const exception& e) {
        cout << "Exception: " << e.what() << endl;
        return 1;
    }

    return 0;
}
