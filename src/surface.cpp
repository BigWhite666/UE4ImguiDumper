#include "main.h"
#include "SysRead.h"
#include "cJSON.h"
#include <iostream>
#include <fstream>
#include <iomanip>
#define GetOffsets(t, m) uint64_t(&(((t*)0)->m))
/**
 *
 */



FName ReadProcessFname(uint64_t address) {
    FName buffer = {0};
    vm_readv(address, &buffer, sizeof(FName));
    if (debug) {
        printf("address=%lx-----buffer.ComparisonIndex= %d ----buffer.Number%d \n", address, buffer.ComparisonIndex,
               buffer.Number);
    }
    return buffer;
}


uint64_t GetClass(uint64_t Address) {
    uint64_t UobjectClass = getPtr64((Address + 0x10));
    if (UobjectClass != NULL) {
        return UobjectClass;
    }
    return uint64_t();
}



FName GetFName(uint64_t Address) {
    FName Name = ReadProcessFname((Address + 0x18));
    if (Name.ComparisonIndex)
        return Name;
    return FName{};

}

std::string GetName_Old(int i) //旧版本算法
{
    uintptr_t G_Names = getPtr64(GNames);
    int Id = (int)(i / (int)0x4000);
    int Idtemp = (int)(i % (int)0x4000);
    auto NamePtr = getPtr64((G_Names + Id * 8));
    auto Name = getPtr64((NamePtr + 8 * Idtemp));
    char name[0x100] = { 0 };
    if (vm_readv((Name + 0xC), name, 0x100)) //0xC需要手动推算，默认是0x10
    {
        return name;
    }
    return std::string();
}

std::string GetName_New(uint32_t index) //新版本算法
{
    unsigned int Block = index >> 16;
    unsigned short int Offset = index & 65535;

    uintptr_t FNamePool = GNames;

    uintptr_t NamePoolChunk = getPtr64(FNamePool+0x40 + (Block * 0x8));

    uintptr_t FNameEntry = NamePoolChunk + (0x2 * Offset);

    short int FNameEntryHeader = getDword(FNameEntry);
    uintptr_t StrPtr = FNameEntry + 0x2;

    int StrLength = FNameEntryHeader >> 6;
    if (debug) {
        printf("FNamePool===%lx\n", FNamePool);
        printf("NamePoolChunk===%lx\n", NamePoolChunk);
        printf("FNameEntry===%lx\n", FNameEntry);
        printf("StrPtr===%lx\n", StrPtr);
    }
    if (StrLength > 0 && StrLength < 250) {
        string name(StrLength, '\0');
        vm_readv(StrPtr, name.data(), StrLength * sizeof(char));
        name.shrink_to_fit();
        return name;
    } else {
        return "None";
    }
}

std::string GetName(uint64_t Address) {
    int FnameComparisonIndex = GetFName(Address).ComparisonIndex; //这里获取的是基类的Gname编号
    if (debug) {
        printf("FnameComparisonIndex=%d\n", FnameComparisonIndex);
    }
    std::string GetName = GetName_New(FnameComparisonIndex); //新算法获取Name
    //std::string GetName = GetName_Old(FnameComparisonIndex); //旧版本算法获取Name

    return GetName;

}

std::string GetClassName(uint64_t Address) {
    uint64_t UobjectClass = GetClass(Address); //获取基类地址
    std::string name = GetName(UobjectClass);
    return name;
}

uint64_t GetOuter(uint64_t Address) {
    uint64_t outer = getPtr64(Address + 0x20);
    if (outer != NULL)
        return outer;
    return uint64_t();
};

std::string GetOuterName(uint64_t Address) {
    auto klass = GetClass(Address);
    if (klass != NULL) {
        std::string temp;
        for (auto outer = GetOuter(Address); outer != 0; outer = GetOuter(outer)) {
            temp = GetName(outer) + "." + temp;
        }

        temp += GetName(Address);  //自己的类名

        return temp;
    }

    return std::string("(null)");
}


static vector<StructureList> foreachAddress(uint64_t Address) {
    std::vector<StructureList> structureList; // 使用std::vector存储输出内容
    for (size_t i = 0; i < 0x300; i+=4) {
        long int Tmp = getPtr64((Address + i));
        //long int Tmp = (Address + i);
        string KlassName = GetClassName(Tmp);
        string outerName = GetOuterName(Tmp);

        StructureList data;
        data.address = (Address + i);
        data.type = KlassName.c_str();
        data.name = outerName.c_str();
        data.offset=i;
        if (KlassName.empty() || outerName.empty())
            continue;

/*        if (outerName.find("null") != outerName.npos || outerName.find("None") != outerName.npos)
            continue;*/
        structureList.push_back(data);

//printf("[%p] %s", (Address + i), Klass.c_str());
    }
    return structureList;
}
// 显示节点信息
static void ShowPlaceholderObject(StructureList data, int uid) {
    // 使用对象的地址作为标识符，确保唯一性。
    ImGui::PushID(data.address);

    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    ImGui::AlignTextToFramePadding();
    long int Pointer = getPtr64(data.address);
    long int P;
    if (GetClass(Pointer) != NULL) {
        P = Pointer;
    } else {
        P = getDword(Pointer);
    }
    float F= getFloat(data.address);
    int D= getDword(data.address);

    bool node_open = ImGui::TreeNode("Tree", "%x——Class:%s————————Name:%s————————%lx : P->%lx F->%f D->%d", data.offset, data.type.c_str(), data.name.c_str(),data.address, P,F,D);

    ImGui::TableSetColumnIndex(0);
    if (node_open) {
        static float placeholder_members[8] = {0.0f, 0.0f, 1.0f, 3.1416f, 100.0f, 999.0f};
        std::vector<StructureList> aaa = foreachAddress(P);
        int i = 0;
        for (const auto &item: aaa) {
            ImGui::PushID(i); // Use field index as identifier.
            ShowPlaceholderObject(item, i);
            ImGui::PopID();
            i++;
        }
        ImGui::TreePop();
    }

    ImGui::PopID();
}

int main(int argc, char *argv[]) {
    // 初始化imgui
    if (!initDraw(false)) {
        return -1;
    }

    Init_touch_config();
    printf("Pid is %d\n", getpid());
    bool flag = true;
    pid = getPID("com.tencent.mf.uam");//包名

    if (pid <= 0) {
        cout << "没有找到进程！" << endl;
        return -1;
    }
    long int libbase = get_module_base(pid, "libUE4.so");
    printf("libbase:%lx\n", libbase);

    long int Matrix = getPtr64(getPtr64(libbase + 0xB2FF440) + 0x0) + 0x9A0;
    long int UWorld = getPtr64(libbase + 0xB3E8A30);
    long int Ulevel = getPtr64(UWorld + 0x30);
    long int Actor = getPtr64(Ulevel + 0x98);
    GNames = libbase + 0xB1D5640;//anqu
    //GNames = libbase + 0xC4F7500;
    printf("UWorld:%lx\n", UWorld);
    printf("GNames:%lx\n", GNames);
    printf("Matrix:%lx\n", Matrix);
    FName aas=GetFName(UWorld);
    printf("GetFName:%d\n", aas.ComparisonIndex);
    string name = GetName_New(6);
    cout << "name\n"<< name << endl;

    float matrix[16] = {0};

    while (flag) {
        // imgui画图开始前调用
        drawBegin();

        static bool UEDump = false;
        //ImGui::ShowDemoWindow(&show_demo_window);

        {
            static float f = 0.0f;
            static int counter = 0;
            static ImVec4 clear_color = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
            //ImGui::Begin("Hello, world!"); // Create a window called "Hello, world!" and append into it.
            ImGui::Begin("UE4Dump工具");
            ImGui::Text("Uworld = %lx", UWorld);
            ImGui::Text("Gname = %lx", GNames);
            ImGui::Checkbox("UEDumpTool", &UEDump);
            if (ImGui::Button("DumpStrings")) {
                // 打开输出文件并重定向标准输出（stdout）到文件
                FILE* outFile = freopen("/storage/emulated/0/string.txt", "w+", stdout);
                if (!outFile) {
                    printf("无法打开输出文件！\n");
                    return -1;
                }
                for (int i = 0; i < 0x1000; ++i) {
                    string className = GetName_New(i);
                    printf("%lx : %s\n", i, className.c_str());
                }
                // 关闭输出文件，并恢复标准输出
                fclose(stdout);
                freopen("CON", "w", stdout);
            }
            if (ImGui::Button("GetGname")) {
                //旧版本
                int i=0;
                while (1){
                    long int gname = getPtr64(libbase+(0x8*i));
                    if (gname != NULL){
                        string Str = GetName(gname);
                        if (Str =="ByteProperty"){
                            break;
                        }
                    }
                    i++;
                }
                printf("%lx",i*8);
                cout << "\n" << endl;
            }
            if (ImGui::Button("exit")) {
                flag = false;
            }

            ImGui::End();
        }
        if (UEDump) {
            ImGui::SetNextWindowSize(ImVec2(1500, 1000), ImGuiCond_FirstUseEver);

            ImGui::Begin("UE4Dump工具");

            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));

            if (ImGui::BeginTable("split", 1, ImGuiTableFlags_BordersOuter | ImGuiTableFlags_Resizable)) {
                std::vector<StructureList> aaa = foreachAddress(UWorld);
                int i = 100000;
                for (const auto &data : aaa) {
                    ImGui::PushID(i);
                    ShowPlaceholderObject(data, i);
                    ImGui::PopID();
                    //ImGui::Separator();
                }
                ImGui::EndTable();
            }
            ImGui::PopStyleVar();

            ImGui::End();

        }

        drawEnd();
        std::this_thread::sleep_for(1ms);
    }
    shutdown();
    touchEnd();
    return 0;
}



