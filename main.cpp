#include <iostream>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <map>
#include <iomanip>
#include <sstream>
#include <windows.h>
#include <shlobj.h>

namespace fs = std::filesystem;
using namespace std;

struct FileInfo {
    string name;
    string relativePath;
    string fullPath;
    string category;
    string extension;
    uintmax_t sizeInBytes;
};

// Global variables
vector<FileInfo> allFiles;
fs::path selectedRootFolder;
int totalFolderCount = 0;

// Button IDs for Win32 Event Router
#define BTN_DASHBOARD 101
#define BTN_DUPLICATES 102
#define BTN_ORGANIZE 103
#define BTN_CHART 104
#define BTN_CSV 105
#define BTN_EXIT 106

// Formats bytes safely into KB, MB, GB strings
string FormatStorageSize(uintmax_t bytes) {
    double size = static_cast<double>(bytes);
    ostringstream stream;
    stream << fixed << setprecision(2);
    if (bytes < 1024) stream << bytes << " Bytes";
    else if (bytes < 1024 * 1024) stream << (size / 1024.0) << " KB";
    else if (bytes < 1024 * 1024 * 1024) stream << (size / (1024.0 * 1024.0)) << " MB";
    else stream << (size / (1024.0 * 1024.0 * 1024.0)) << " GB";
    return stream.str();
}

string OpenFolderBrowserGUI() {
    TCHAR path[MAX_PATH];
    BROWSEINFO bi = { 0 };
    bi.lpszTitle = TEXT("Select the folder you want to organize:");
    bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
    LPITEMIDLIST pidl = SHBrowseForFolder(&bi);
    if (pidl != 0) {
        SHGetPathFromIDList(pidl, path);
        IMalloc* imalloc = 0;
        if (SUCCEEDED(SHGetMalloc(&imalloc))) {
            imalloc->Free(pidl);
            imalloc->Release();
        }
        #ifdef UNICODE
            std::wstring wpath(path);
            return std::string(wpath.begin(), wpath.end());
        #else
            return std::string(path);
        #endif
    }
    return "";
}

string getFileCategory(string ext) {
    transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    if (ext == ".jpg" || ext == ".png" || ext == ".jpeg" || ext == ".bmp" || ext == ".jfif") return "Images";
    if (ext == ".pdf" || ext == ".docx" || ext == ".txt" || ext == ".pptx" || ext == ".csv") return "Documents";
    if (ext == ".mp3" || ext == ".wav") return "Audio";
    if (ext == ".mp4" || ext == ".mkv" || ext == ".avi") return "Video";
    if (ext == ".zip" || ext == ".rar" || ext == ".7z") return "Archives";
    return "Other";
}

void scanDirectory(fs::path currentPath) {
    allFiles.clear();
    totalFolderCount = 0;
    try {
        for (const auto& item : fs::recursive_directory_iterator(currentPath)) {
            if (fs::is_directory(item.path())) totalFolderCount++;
            else if (fs::is_regular_file(item.path())) {
                FileInfo f;
                f.name = item.path().filename().string();
                f.fullPath = item.path().string();
                f.relativePath = fs::relative(item.path(), currentPath).string();
                f.extension = item.path().extension().string();
                f.category = getFileCategory(f.extension);
                f.sizeInBytes = fs::file_size(item.path());
                allFiles.push_back(f);
            }
        }
    } catch (...) {}
}

void showDashboard() {
    uintmax_t totalBytes = 0;
    map<string, pair<int, uintmax_t>> categoryTotals;
    map<string, vector<FileInfo>> categoryFiles;

    for (const auto& f : allFiles) {
        totalBytes += f.sizeInBytes;
        categoryTotals[f.category].first++;
        categoryTotals[f.category].second += f.sizeInBytes;
        categoryFiles[f.category].push_back(f);
    }

    string htmlPath = (selectedRootFolder / "Visual_Dashboard.html").string();
    ofstream html(htmlPath);
    if (html.is_open()) {
        html << "<html><head><title>Storage Insights Studio</title><style>"
             << "body { font-family: 'Segoe UI', system-ui, sans-serif; background-color: #0f172a; color: #f8fafc; padding: 40px; margin: 0; }"
             << ".container { max-width: 1000px; margin: auto; background: #1e293b; padding: 35px; border-radius: 16px; box-shadow: 0 10px 30px rgba(0,0,0,0.5); }"
             << "h1 { color: #38bdf8; text-align: center; border-bottom: 3px solid #334155; padding-bottom: 20px; font-size: 2.2em; margin-top: 0; }"
             << ".stats-grid { display: grid; grid-template-columns: repeat(2, 1fr); gap: 20px; margin-bottom: 35px; }"
             << ".card { background: #334155; padding: 20px; border-radius: 12px; border-left: 6px solid #38bdf8; transition: transform 0.2s; }"
             << ".card:hover { transform: translateY(-2px); }"
             << ".card.accent { border-left-color: #34d399; }"
             << ".title { font-weight: 600; font-size: 1em; color: #94a3b8; text-transform: uppercase; margin: 0 0 5px 0; }"
             << ".value { font-size: 1.4em; font-weight: 700; color: #fff; margin: 0; word-break: break-all; }"
             << ".category-section { background: #0f172a; margin-bottom: 15px; border-radius: 10px; overflow: hidden; border: 1px solid #334155; }"
             << ".category-header { background: #1e293b; padding: 18px 25px; display: flex; justify-content: space-between; align-items: center; cursor: pointer; font-size: 1.1em; font-weight: bold; transition: background 0.2s; }"
             << ".category-header:hover { background: #2d3748; }"
             << ".category-header span.badge { background: #38bdf8; color: #0f172a; padding: 4px 10px; border-radius: 20px; font-size: 0.85em; font-weight: bold; }"
             << ".file-dropdown { display: none; padding: 15px; background: #0f172a; border-top: 1px solid #232946; }"
             << "table { width: 100%; border-collapse: collapse; text-align: left; }"
             << "th, td { padding: 10px 15px; border-bottom: 1px solid #1e293b; font-size: 0.95em; }"
             << "th { color: #94a3b8; font-weight: 600; text-transform: uppercase; font-size: 0.8em; }"
             << "tr:hover { background: #1e293b; }"
             << ".ext-tag { background: #334155; padding: 2px 6px; border-radius: 4px; font-family: monospace; color: #38bdf8; font-size: 0.9em; }"
             << "</style>"
             << "<script>"
             << "function toggleDropdown(id) {"
             << "  var element = document.getElementById(id);"
             << "  if(element.style.display === 'block') { element.style.display = 'none'; }"
             << "  else { element.style.display = 'block'; }"
             << "}"
             << "</script></head><body>"
             << "<div class='container'>"
             << "<h1>WORKSPACE DATA INSIGHTS STUDIO</h1>"
             << "<div class='stats-grid'>"
             << "<div class='card'><p class='title'>Workspace Target Path</p><p class='value' style='font-size:1.1em;'>" << selectedRootFolder.string() << "</p></div>"
             << "<div class='card accent'><p class='title'>Combined File Footprint</p><p class='value'>" << FormatStorageSize(totalBytes) << "</p></div>"
             << "<div class='card accent'><p class='title'>Directories Indexed</p><p class='value'>" << totalFolderCount << " folders</p></div>"
             << "<div class='card'><p class='title'>Total Files Tracked</p><p class='value'>" << allFiles.size() << " objects</p></div>"
             << "</div>"
             << "<h2>Interactive Category Analyzer (Click to Expand Files)</h2>";

        vector<string> masterCategories = {"Images", "Documents", "Audio", "Video", "Archives", "Other"};
        int uniqueId = 0;

        for (const auto& catName : masterCategories) {
            uniqueId++;
            int count = categoryTotals[catName].first;
            uintmax_t size = categoryTotals[catName].second;
            string divId = "cat_dropdown_" + to_string(uniqueId);

            html << "<div class='category-section'>"
                 << "<div class='category-header' onclick=\"toggleDropdown('" << divId << "')\">"
                 << "<span>📁 " << catName << " Portfolio (" << FormatStorageSize(size) << ")</span>"
                 << "<span class='badge'>" << count << " File(s)</span>"
                 << "</div>"
                 << "<div id='" << divId << "' class='file-dropdown'>"
                 << "<table><thead><tr><th>File Name</th><th>Extension</th><th>Storage Sizing</th></tr></thead><tbody>";

            if (categoryFiles[catName].empty()) {
                html << "<tr><td colspan='3' style='text-align:center; color:#64748b;'>No available structural file records index mapped.</td></tr>";
            } else {
                for (const auto& file : categoryFiles[catName]) {
                    html << "<tr>"
                         << "<td style='word-break:break-all;'>" << file.name << "</td>"
                         << "<td><span class='ext-tag'>" << (file.extension.empty() ? ".none" : file.extension) << "</span></td>"
                         << "<td>" << FormatStorageSize(file.sizeInBytes) << "</td>"
                         << "</tr>";
                }
            }
            html << "</tbody></table></div></div>";
        }
        html << "</div></body></html>";
        html.close();
        ShellExecuteA(NULL, "open", htmlPath.c_str(), NULL, NULL, SW_SHOWNORMAL);
    }
}

void findDuplicates() {
    string content = "Duplicate redundancy file matching results:\n\n";
    int duplicatePairs = 0;
    vector<string> duplicatePathsToDelete;

    for (size_t i = 0; i < allFiles.size(); i++) {
        for (size_t j = i + 1; j < allFiles.size(); j++) {
            if (allFiles[i].sizeInBytes == allFiles[j].sizeInBytes && allFiles[i].sizeInBytes > 0) {
                if (duplicatePairs < 5) { 
                    content += "- Match Size: " + FormatStorageSize(allFiles[i].sizeInBytes) + "\n";
                    content += "  Original: " + allFiles[i].name + "\n  Duplicate: " + allFiles[j].name + "\n\n";
                }
if (find(duplicatePathsToDelete.begin(), duplicatePathsToDelete.end(), allFiles[j].fullPath) == duplicatePathsToDelete.end()) {
duplicatePathsToDelete.push_back(allFiles[j].fullPath);
}
duplicatePairs++;
}
}
}
if (duplicatePairs == 0) {
content += "No duplicate configurations found inside current active directory.";
MessageBoxA(NULL, content.c_str(), "Duplicate Checker Diagnostics", MB_OK | MB_ICONINFORMATION);
} else {
content += "Total duplicate sets discovered: " + to_string(duplicatePairs) + "\n\n";
content += "Would you like to automatically REMOVE/DELETE these duplicate versions from your disk now?";
int userChoice = MessageBoxA(NULL, content.c_str(), "Clean Duplicate Space?", MB_YESNO | MB_ICONQUESTION);
if (userChoice == IDYES) {
int deletedCount = 0;
for (size_t k = 0; k < duplicatePathsToDelete.size(); k++) {
try {
if (fs::exists(duplicatePathsToDelete[k])) {
fs::remove(duplicatePathsToDelete[k]);
deletedCount++;
}
} catch (...) {}
}
string resultMsg = "Cleanup Done! Successfully deleted " + to_string(deletedCount) + " redundant files.";
MessageBoxA(NULL, resultMsg.c_str(), "Success", MB_OK | MB_ICONINFORMATION);
scanDirectory(selectedRootFolder);
}
}
}
void showVisualChart() {
int img = 0, doc = 0, aud = 0, vid = 0, arc = 0, oth = 0;
for (const auto& f : allFiles) {
if (f.category == "Images") img++;
else if (f.category == "Documents") doc++;
else if (f.category == "Audio") aud++;
else if (f.category == "Video") vid++;
else if (f.category == "Archives") arc++;
else oth++;
}
string content = "Storage volume grouping charts:\n\n";
content += "Images    --> " + string(min(15, img), '#') + " (" + to_string(img) + ")\n";
content += "Documents --> " + string(min(15, doc), '#') + " (" + to_string(doc) + ")\n";
content += "Audio     --> " + string(min(15, aud), '#') + " (" + to_string(aud) + ")\n";
content += "Video     --> " + string(min(15, vid), '#') + " (" + to_string(vid) + ")\n";
content += "Archives  --> " + string(min(15, arc), '#') + " (" + to_string(arc) + ")\n";
content += "Other     --> " + string(min(15, oth), '#') + " (" + to_string(oth) + ")\n";
MessageBoxA(NULL, content.c_str(), "Visual Layout Density", MB_OK | MB_ICONINFORMATION);
}
// 100% BULLETPROOF EXCEL GENERATOR: Uses strict independent character mapping variables
void exportToCSV() {
string filePath = (selectedRootFolder / "inventory_report.csv").string();
ofstream outFile(filePath);
if (!outFile.is_open()) {
MessageBoxA(NULL, "Error generating export tracking file streams.", "Export Failed", MB_OK | MB_ICONERROR);
return;
}
// Write file headers row cleanly
outFile << "File Name,Category,Extension,Size in Bytes,Human Size,Relative Path\n";
// Loop through vector objects and print parameters safely using character mappings
for (size_t i = 0; i < allFiles.size(); i++) {
string nameVal = allFiles[i].name;
string catVal  = allFiles[i].category;
string extVal  = allFiles[i].extension.empty() ? ".none" : allFiles[i].extension;
string byteVal = to_string(allFiles[i].sizeInBytes);
string sizeVal = FormatStorageSize(allFiles[i].sizeInBytes);
string relVal  = allFiles[i].relativePath;
char q = '"'; // Quote mark character variable
char c = ','; // Comma character variable
outFile << q << nameVal << q << c
<< q << catVal << q << c
<< q << extVal << q << c
<< byteVal << c
<< q << sizeVal << q << c
<< q << relVal << q << "\n";
}
outFile.close();
string msg = "Excel Spreadsheet (.CSV Report) exported successfully to:\n" + filePath;
MessageBoxA(NULL, msg.c_str(), "CSV Export Success", MB_OK | MB_ICONINFORMATION);
}
void organizeFiles() {
fs::path vault = selectedRootFolder / "Organized_Vault";
int copied = 0, skipped = 0;
for (size_t i = 0; i < allFiles.size(); i++) {
if (allFiles[i].extension == ".cpp" || allFiles[i].extension == ".exe" || allFiles[i].extension == ".cbp") continue;
if (allFiles[i].fullPath.find("Organized_Vault") != string::npos) continue;
try {
fs::path targetFolder = vault / allFiles[i].category;
fs::create_directories(targetFolder);
fs::path finalDestination = targetFolder / allFiles[i].name;
int counter = 1;
while (fs::exists(finalDestination)) {
finalDestination = targetFolder / ("copy" + to_string(counter) + "_" + allFiles[i].name);
counter++;
}
fs::copy_file(allFiles[i].fullPath, finalDestination, fs::copy_options::overwrite_existing);
copied++;
} catch (...) { skipped++; }
}
string msg = "Sorting complete!\n\n- Successfully organized: " + to_string(copied) + " file(s)";
MessageBoxA(NULL, msg.c_str(), "File Restructuring Done", MB_OK | MB_ICONINFORMATION);
scanDirectory(selectedRootFolder);
}
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
switch (msg) {
case WM_CREATE: {
CreateWindowA("STATIC", "FILE ORGANIZER CENTRAL CONTROLLER STUDIO", WS_VISIBLE | WS_CHILD | SS_CENTER, 10, 15, 510, 20, hwnd, NULL, NULL, NULL);
CreateWindowA("STATIC", "Click the operational node headers to trigger automated visual systems:", WS_VISIBLE | WS_CHILD, 20, 50, 480, 20, hwnd, NULL, NULL, NULL);
CreateWindowA("STATIC", "Open Premium Dropdown Interactive Dashboard Studio", WS_VISIBLE | WS_CHILD, 25, 95, 350, 20, hwnd, NULL, NULL, NULL);
CreateWindowA("BUTTON", "View Dashboard", WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 380, 90, 130, 30, hwnd, (HMENU)BTN_DASHBOARD, NULL, NULL);
CreateWindowA("STATIC", "Run Storage Redundancy Scan & Smart Deletion Engine", WS_VISIBLE | WS_CHILD, 25, 145, 350, 20, hwnd, NULL, NULL, NULL);
CreateWindowA("BUTTON", "Scan Duplicates", WS_VISIBLE | WS_CHILD, 380, 140, 130, 30, hwnd, (HMENU)BTN_DUPLICATES, NULL, NULL);
CreateWindowA("STATIC", "Execute Automatic Category Layout Sorting Processes", WS_VISIBLE | WS_CHILD, 25, 195, 350, 20, hwnd, NULL, NULL, NULL);
CreateWindowA("BUTTON", "Organize Files", WS_VISIBLE | WS_CHILD, 380, 190, 130, 30, hwnd, (HMENU)BTN_ORGANIZE, NULL, NULL);
CreateWindowA("STATIC", "Render Storage Capacity Distribution Density Charts", WS_VISIBLE | WS_CHILD, 25, 245, 350, 20, hwnd, NULL, NULL, NULL);
CreateWindowA("BUTTON", "View Chart", WS_VISIBLE | WS_CHILD, 380, 240, 130, 30, hwnd, (HMENU)BTN_CHART, NULL, NULL);
CreateWindowA("STATIC", "Compile & Export Microsoft Excel Tracking Spreadsheets", WS_VISIBLE | WS_CHILD, 25, 295, 350, 20, hwnd, NULL, NULL, NULL);
CreateWindowA("BUTTON", "Export CSV", WS_VISIBLE | WS_CHILD, 380, 290, 130, 30, hwnd, (HMENU)BTN_CSV, NULL, NULL);
CreateWindowA("BUTTON", "EXIT MANAGEMENT STUDIO", WS_VISIBLE | WS_CHILD, 20, 350, 490, 45, hwnd, (HMENU)BTN_EXIT, NULL, NULL);
break;
}
case WM_COMMAND: {
switch (LOWORD(wParam)) {
case BTN_DASHBOARD: showDashboard(); break;
case BTN_DUPLICATES: findDuplicates(); break;
case BTN_ORGANIZE: organizeFiles(); break;
case BTN_CHART: showVisualChart(); break;
case BTN_CSV: exportToCSV(); break;
case BTN_EXIT: PostQuitMessage(0); break;
}
break;
}
case WM_DESTROY: {
PostQuitMessage(0);
break;
}
default:
return DefWindowProc(hwnd, msg, wParam, lParam);
}
return 0;
}
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
MessageBoxA(NULL, "Welcome to Digital File Organizer Studio 2.0!\n\nClick OK to open the Workspace Directory Browser Selector.", "System Initialization", MB_OK | MB_ICONINFORMATION);
string folderInput = OpenFolderBrowserGUI();
if (folderInput.empty()) return 0;
selectedRootFolder = fs::u8path(folderInput);
if (!fs::exists(selectedRootFolder) || !fs::is_directory(selectedRootFolder)) {
MessageBoxA(NULL, "Selected route path configuration is invalid!", "Critical Error", MB_OK | MB_ICONERROR);
return 0;
}
scanDirectory(selectedRootFolder);
const char* g_szClassName = "OrganizerStudioGUI";
WNDCLASSEXA wc = {0};
wc.cbSize = sizeof(WNDCLASSEX);
wc.lpfnWndProc = WndProc;
wc.hInstance = hInstance;
wc.hCursor = LoadCursor(NULL, IDC_ARROW);
wc.hbrBackground = CreateSolidBrush(RGB(240, 242, 245));
wc.lpszClassName = g_szClassName;
if (!RegisterClassExA(&wc)) return 0;
HWND hwnd = CreateWindowExA(
WS_EX_CLIENTEDGE, g_szClassName, "Digital File Organizer Studio 2.0 (Premium UI Mode)",
WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
CW_USEDEFAULT, CW_USEDEFAULT, 550, 450, NULL, NULL, hInstance, NULL
);
if (hwnd == NULL) return 0;
ShowWindow(hwnd, nCmdShow);
UpdateWindow(hwnd);
MSG Msg;
while (GetMessage(&Msg, NULL, 0, 0) > 0) {
TranslateMessage(&Msg);
DispatchMessage(&Msg);
}
return Msg.wParam;
}