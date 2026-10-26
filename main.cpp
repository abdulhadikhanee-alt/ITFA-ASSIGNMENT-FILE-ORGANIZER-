#include <iostream>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <iomanip>
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

void ShowBeautifulGui(string title, string contentText) {
    MessageBoxA(NULL, contentText.c_str(), title.c_str(), MB_OK | MB_ICONINFORMATION);
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
    if (ext == ".jpg" || ext == ".png" || ext == ".jpeg" || ext == ".bmp") return "Images";
    if (ext == ".pdf" || ext == ".docx" || ext == ".txt" || ext == ".pptx") return "Documents";
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
            if (fs::is_directory(item.path())) {
                totalFolderCount++;
            }
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

// Module 2: Generates an advanced, highly visual, colorful HTML dashboard view
void showDashboard() {
    uintmax_t totalBytes = 0;
    int img = 0, doc = 0, aud = 0, vid = 0, arc = 0, oth = 0;

    for (const auto& f : allFiles) {
        totalBytes += f.sizeInBytes;
        if (f.category == "Images") img++;
        else if (f.category == "Documents") doc++;
        else if (f.category == "Audio") aud++; // Fixed typo here
        else if (f.category == "Video") vid++;
        else if (f.category == "Archives") arc++;
        else oth++;
    }

    // Save dashboard to a clean HTML styled file wrapper
    string htmlPath = (selectedRootFolder / "Visual_Dashboard.html").string();
    ofstream html(htmlPath);

    if (html.is_open()) {
        html << "<html><head><style>"
             << "body { font-family: 'Segoe UI', sans-serif; background-color: #1a1a2e; color: #fff; padding: 30px; }"
             << ".container { max-width: 900px; margin: auto; background: #161625; padding: 25px; border-radius: 12px; box-shadow: 0 8px 24px rgba(0,0,0,0.3); }"
             << "h1 { color: #4ecca3; text-align: center; border-bottom: 2px solid #232946; padding-bottom: 15px; }"
             << ".stats-grid { display: grid; grid-template-columns: 1fr 1fr; gap: 20px; margin-bottom: 25px; }"
             << ".card { background: #232946; padding: 15px; border-radius: 8px; border-left: 5px solid #4ecca3; }"
             << ".card.blue { border-left-color: #00fff0; }"
             << ".category-list { background: #1f1f38; padding: 15px; border-radius: 8px; }"
             << ".item { display: flex; justify-content: space-between; padding: 8px 0; border-bottom: 1px solid #2d2d44; }"
             << ".title { font-weight: bold; color: #4ecca3; }"
             << "</style></head><body>"
             << "<div class='container'>"
             << "<h1>DIGITAL STORAGE ANALYTICS SUITE</h1>"
             << "<div class='stats-grid'>"
             << "<div class='card'><p class='title'>Workspace Target Path</p><p>" << selectedRootFolder.string() << "</p></div>"
             << "<div class='card blue'><p class='title'>Total Space Tracked</p><p>" << (totalBytes / (1024 * 1024)) << " MB</p></div>"
             << "<div class='card'><p class='title'>Total Directories Index</p><p>" << totalFolderCount << " folders</p></div>"
             << "<div class='card blue'><p class='title'>Total Files Tracked</p><p>" << allFiles.size() << " objects</p></div>"
             << "</div>"
             << "<h2>Category File Volume</h2>"
             << "<div class='category-list'>"
             << "<div class='item'><span>Images Directory</span><span>" << img << " file(s)</span></div>"
             << "<div class='item'><span>Documents Directory</span><span>" << doc << " file(s)</span></div>"
             << "<div class='item'><span>Audio Tracks</span><span>" << aud << " file(s)</span></div>"
             << "<div class='item'><span>Video Productions</span><span>" << vid << " file(s)</span></div>"
             << "<div class='item'><span>Compressed Archives</span><span>" << arc << " file(s)</span></div>"
             << "<div class='item'><span>Other Extensions</span><span>" << oth << " file(s)</span></div>"
             << "</div>"
             << "</div></body></html>";
        html.close();

        // Instruct Windows to launch the colorful dashboard in their browser instantly
        ShellExecuteA(NULL, "open", htmlPath.c_str(), NULL, NULL, SW_SHOWNORMAL);
    }

    // Backup widescreen dialog output box
    string content = "========================================================\n";
    content += "                   SYSTEM STORAGE DASHBOARD ANALYTICS                  \n";
    content += "========================================================\n\n";
    content += " [*] Target Workspace Path :\n     " + selectedRootFolder.string() + "\n\n";
    content += " [+] Total Sub-Folders Found : " + to_string(totalFolderCount) + "\n";
    content += " [+] Total Files Discovered  : " + to_string(allFiles.size()) + "\n";
    content += " [+] Total Allocated Storage  : " + to_string(totalBytes / (1024 * 1024)) + " MB\n\n";
    content += "--------------------------------------------------------------------------------------\n";
    content += "   FILE CATEGORY DISTRIBUTION TOTALS\n";
    content += "--------------------------------------------------------------------------------------\n";
    content += "    [-] Images Directory   --> " + to_string(img) + " file(s)\n";
    content += "    [-] Documents Vault    --> " + to_string(doc) + " file(s)\n";
    content += "    [-] Audio Playlists    --> " + to_string(aud) + " file(s)\n";
    content += "    [-] Video Productions  --> " + to_string(vid) + " file(s)\n";
    content += "    [-] Archives Packets   --> " + to_string(arc) + " file(s)\n";
    content += "    [-] Miscellaneous Data --> " + to_string(oth) + " file(s)\n\n";
    content += " (Colorful Advanced Web Dashboard has been opened on your screen!)";

    ShowBeautifulGui("Dashboard Data View", content);
}

void findDuplicates() {
    string content = "========================================================\n";
    content += "                      DUPLICATE REDUNDANCY SCANNER                      \n";
    content += "========================================================\n\n";
    int duplicatePairs = 0;

    for (size_t i = 0; i < allFiles.size(); i++) {
        for (size_t j = i + 1; j < allFiles.size(); j++) {
            if (allFiles[i].sizeInBytes == allFiles[j].sizeInBytes && allFiles[i].sizeInBytes > 0) {
                if (duplicatePairs < 5) { 
                    content += " [!] [Duplicate Conflict Match]\n";
                    content += "      Allocated Size : " + to_string(allFiles[i].sizeInBytes / 1024) + " KB\n";
                    content += "      File 1 : " + allFiles[i].name + "\n";
                    content += "      File 2 : " + allFiles[j].name + "\n\n";
                }
                duplicatePairs++;
            }
        }
    }

    if (duplicatePairs == 0) {
        content += " [+] Integrity Check Complete! No duplicate clusters detected.";
    } else {
        content += "--------------------------------------------------------------------------------------\n";
        content += " Total redundant duplicate pairs discovered: " + to_string(duplicatePairs);
    }
    ShowBeautifulGui("Data Integrity Diagnostics", content);
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

    string content = "========================================================\n";
    content += "                    STORAGE DENSITY RATIO DATA VISUALIZER                 \n";
    content += "========================================================\n\n";
    content += "  Images      | " + string(min(15, img), '#') + " (" + to_string(img) + ")\n";
    content += "  Documents   | " + string(min(15, doc), '#') + " (" + to_string(doc) + ")\n";
content += "  Audio       | " + string(min(15, aud), '#') + " (" + to_string(aud) + ")\n";
content += "  Video       | " + string(min(15, vid), '#') + " (" + to_string(vid) + ")\n";
content += "  Archives    | " + string(min(15, arc), '#') + " (" + to_string(arc) + ")\n";
content += "  Other       | " + string(min(15, oth), '#') + " (" + to_string(oth) + ")\n\n";
content += "--------------------------------------------------------------------------------------\n";
content += "  (Layout Indicator: Each '#' represents 1 unique file inside tracking structures)";
ShowBeautifulGui("Visualizer Interface", content);
}
void exportToCSV() {
string filePath = (selectedRootFolder / "inventory_report.csv").string();
ofstream outFile(filePath);
if (!outFile.is_open()) {
MessageBoxA(NULL, "Error: Could not instantiate output logs.", "Export Failed", MB_OK | MB_ICONERROR);
return;
}
outFile << "File Name,Category,Extension,Size in Bytes,Relative Path\n";
for (const auto& f : allFiles) {
outFile << """ << f.name << ""," << """ << f.category << ""," << """ << f.extension << ""," << f.sizeInBytes << "," << """ << f.relativePath << ""\n";
}
outFile.close();
string msg = "[+] SUCCESS!\n\nYour tracking report has been exported to:\n" + filePath;
MessageBoxA(NULL, msg.c_str(), "CSV Report Export Success", MB_OK | MB_ICONINFORMATION);
}
void organizeFiles() {
int msgBoxID = MessageBoxA(NULL, "Do you want to run the automatic category folder organization now?", "Confirm Options", MB_YESNO | MB_ICONQUESTION);
if (msgBoxID == IDNO) return;
fs::path vault = selectedRootFolder / "Organized_Vault";
int copied = 0;
int skipped = 0;
for (const auto& f : allFiles) {
if (f.extension == ".cpp" || f.extension == ".exe" || f.extension == ".cbp") continue;
if (f.fullPath.find("Organized_Vault") != string::npos) continue;
try {
fs::path targetFolder = vault / f.category;
fs::create_directories(targetFolder);
fs::path finalDestination = targetFolder / f.name;
int counter = 1;
while (fs::exists(finalDestination)) {
finalDestination = targetFolder / ("copy" + to_string(counter) + "_" + f.name);
counter++;
}
fs::copy_file(f.fullPath, finalDestination, fs::copy_options::overwrite_existing);
copied++;
}
catch (...) {
skipped++;
}
}
string finalMsg = " [+] FILE CATEGORIZATION COMPLETE!\n\n";
finalMsg += " [>] Total files restructured safely: " + to_string(copied) + " items\n";
if (skipped > 0) {
finalMsg += " [>] Conflicting items skipped safely: " + to_string(skipped);
}
MessageBoxA(NULL, finalMsg.c_str(), "Sorting Complete", MB_OK | MB_ICONINFORMATION);
scanDirectory(selectedRootFolder);
}
int main() {
MessageBoxA(NULL, "Welcome to Digital File Organizer Studio 2.0!\n\nClick OK to open the Workspace Folder Selector browser.", "Digital File Organizer", MB_OK | MB_ICONINFORMATION);
string folderInput = OpenFolderBrowserGUI();
if (folderInput.empty()) {
MessageBoxA(NULL, "No directory target specified. Exiting application.", "Session Aborted", MB_OK | MB_ICONWARNING);
return 0;
}
selectedRootFolder = fs::u8path(folderInput);
if (!fs::exists(selectedRootFolder) || !fs::is_directory(selectedRootFolder)) {
MessageBoxA(NULL, "The target directory structure path is invalid!", "Error", MB_OK | MB_ICONERROR);
return 0;
}
scanDirectory(selectedRootFolder);
string syncMsg = "[+] Synchronization Finished!\n\nTarget directory successfully indexed.\nTotal files: " + to_string(allFiles.size()) + " items.";
MessageBoxA(NULL, syncMsg.c_str(), "Workspace Synced", MB_OK | MB_ICONINFORMATION);
while (true) {
string menuPrompt = "========================================================\n";
menuPrompt += "                     FILE ORGANIZER GUI MASTER CONTROL PANEL                  \n";
menuPrompt += "========================================================\n\n";
menuPrompt += "  Active Directory path location:\n  " + selectedRootFolder.string() + "\n\n";
menuPrompt += "  Choose your action branch execution by clicking controls below:\n\n";
menuPrompt += "   [YES]      ==>  Open Colorful Web Dashboard Analytics view\n\n";
menuPrompt += "   [NO]       ==>  Open Advanced Utilities (Duplicates, Sorting, CSV Reports)\n\n";
menuPrompt += "   [CANCEL]   ==>  Safe Session Close Sequence\n";
menuPrompt += "========================================================";
int mainChoice = MessageBoxA(NULL, menuPrompt.c_str(), "Main Executive Controller", MB_YESNOCANCEL | MB_ICONQUESTION);
if (mainChoice == IDYES) {
showDashboard();
}
else if (mainChoice == IDNO) {
string subPrompt = "========================================================\n";
subPrompt += "                      ADVANCED SYSTEM OPERATIONS SUITE                       \n";
subPrompt += "========================================================\n\n";
subPrompt += "  Select sub-routine deployment thread configuration:\n\n";
subPrompt += "   [YES]      ==>  Run Duplicate Checker Integrity Scan\n\n";
subPrompt += "   [NO]       ==>  Execute File Category Restructuring (Organize Files)\n\n";
subPrompt += "   [CANCEL]   ==>  Compile Inventory Spreadsheets & View Graphs\n";
subPrompt += "========================================================";
int subChoice = MessageBoxA(NULL, subPrompt.c_str(), "Advanced Operations Toolset", MB_YESNOCANCEL | MB_ICONASTERISK);
if (subChoice == IDYES) {
findDuplicates();
}
else if (subChoice == IDNO) {
organizeFiles();
}
else if (subChoice == IDCANCEL) {
exportToCSV();
showVisualChart();
}
}
else if (mainChoice == IDCANCEL) {
MessageBoxA(NULL, "Thank you for using Digital File Organizer Studio.\n\nApplication Session Closed. Allah Hafiz!", "Goodbye", MB_OK | MB_ICONINFORMATION);
break;
}
}
return 0;
}