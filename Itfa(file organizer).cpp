#include <iostream>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <iomanip>

namespace fs = std::filesystem;
using namespace std;

// Har file ka data store karne ke liye simple structure
struct FileInfo {
    string name;
    string relativePath;
    string fullPath;
    string category;
    string extension;
    uintmax_t sizeInBytes;
};

// Global variables (Viva me explain karna sabse aasan)
vector<FileInfo> allFiles;
fs::path selectedRootFolder;
int totalFolderCount = 0;

// Simple function: Extension dekh kar category batana
string getFileCategory(string ext) {
    // Extension ko lowercase banana taake .JPG aur .jpg dono chalein
    transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

    if (ext == ".jpg" || ext == ".png" || ext == ".jpeg" || ext == ".bmp") return "Images";
    if (ext == ".pdf" || ext == ".docx" || ext == ".txt" || ext == ".pptx") return "Documents";
    if (ext == ".mp3" || ext == ".wav") return "Audio";
    if (ext == ".mp4" || ext == ".mkv" || ext == ".avi") return "Video";
    if (ext == ".zip" || ext == ".rar" || ext == ".7z") return "Archives";
    return "Other";
}

// Module 1: Folder aur subfolders scan karna (Recursive)
void scanDirectory(fs::path currentPath) {
    allFiles.clear();
    totalFolderCount = 0;

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
}

// Module 2: Dashboard aur Top 10 Files
void showDashboard() {
    uintmax_t totalBytes = 0;
    int imgCount = 0, docCount = 0, audCount = 0, vidCount = 0, arcCount = 0, othCount = 0;

    for (const auto& f : allFiles) {
        totalBytes += f.sizeInBytes;
        if (f.category == "Images") imgCount++;
        else if (f.category == "Documents") docCount++;
        else if (f.category == "Audio") audCount++;
        else if (f.category == "Video") vidCount++;
        else if (f.category == "Archives") arcCount++;
        else othCount++;
    }

    cout << "\n==================== DASHBOARD ====================\n";
    cout << "Selected Folder: " << selectedRootFolder.string() << "\n";
    cout << "Total Folders: " << totalFolderCount << "\n";
    cout << "Total Files: " << allFiles.size() << "\n";
    cout << "Total Storage: " << (totalBytes / 1024) << " KB (" << (totalBytes / (1024 * 1024)) << " MB)\n";
    cout << "----------------------------------------------------\n";
    cout << "Category Breakdown:\n";
    cout << "  Images:    " << imgCount << " file(s)\n";
    cout << "  Documents: " << docCount << " file(s)\n";
    cout << "  Audio:     " << audCount << " file(s)\n";
    cout << "  Video:     " << vidCount << " file(s)\n";
    cout << "  Archives:  " << arcCount << " file(s)\n";
    cout << "  Other:     " << othCount << " file(s)\n";
    cout << "----------------------------------------------------\n";

    // Top 10 largest files using simple sorting
    vector<FileInfo> tempFiles = allFiles;
    sort(tempFiles.begin(), tempFiles.end(), [](const FileInfo& a, const FileInfo& b) {
        return a.sizeInBytes > b.sizeInBytes;
    });

    cout << "Top 10 Largest Files:\n";
    int limit = min(10, (int)tempFiles.size());
    for (int i = 0; i < limit; i++) {
        cout << (i + 1) << ". " << tempFiles[i].name
             << " | " << (tempFiles[i].sizeInBytes / 1024) << " KB"
             << " | " << tempFiles[i].category << "\n";
    }
    cout << "====================================================\n";
}

// Module 3: Search Files
void searchFiles() {
    cout << "\nEnter file name or keyword to search: ";
    string query;
    getline(cin, query);

    cout << "\nSearch Results:\n";
    int found = 0;
    for (const auto& f : allFiles) {
        if (f.name.find(query) != string::npos) {
            cout << "-> " << f.name << " (" << (f.sizeInBytes / 1024) << " KB) at " << f.relativePath << "\n";
            found++;
        }
    }
    cout << "Total found: " << found << "\n";
}

// Module 4: Duplicate Files Finder (Exact Size Match)
void findDuplicates() {
    cout << "\n================= DUPLICATE CHECKER ================\n";
    cout << "Logic: Files with exact same size in bytes are compared.\n";
    cout << "(Equal names do not prove duplication; equal sizes do.)\n";
    cout << "----------------------------------------------------\n";

    int duplicatePairs = 0;
    for (size_t i = 0; i < allFiles.size(); i++) {
        for (size_t j = i + 1; j < allFiles.size(); j++) {
            if (allFiles[i].sizeInBytes == allFiles[j].sizeInBytes && allFiles[i].sizeInBytes > 0) {
                cout << "[Possible Duplicate] Size: " << (allFiles[i].sizeInBytes / 1024) << " KB\n";
                cout << "  File 1: " << allFiles[i].relativePath << "\n";
                cout << "  File 2: " << allFiles[j].relativePath << "\n";
                duplicatePairs++;
            }
        }
    }

    if (duplicatePairs == 0) {
        cout << "No duplicate files found.\n";
    } else {
        cout << "\nTotal duplicate pairs detected: " << duplicatePairs << "\n";
        cout << "Note: No files were deleted automatically (Data Safety).\n";
    }
    cout << "====================================================\n";
}

// Module 5: Visual Bar Chart (Console Graphs)
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

    cout << "\n================ VISUAL FILE SUMMARY ===============\n";
    cout << "Images    | " << string(img / 2, '#') << " (" << img << ")\n";
    cout << "Documents | " << string(doc / 2, '#') << " (" << doc << ")\n";
    cout << "Audio     | " << string(aud / 2, '#') << " (" << aud << ")\n";
    cout << "Video     | " << string(vid / 2, '#') << " (" << vid << ")\n";
    cout << "Archives  | " << string(arc / 2, '#') << " (" << arc << ")\n";
    cout << "Other     | " << string(oth / 2, '#') << " (" << oth << ")\n";
    cout << "(Scale: 1 '#' represents 2 files)\n";
    cout << "====================================================\n";
}

// Module 6: Export to CSV (Excel File)
void exportToCSV() {
    string filePath = (selectedRootFolder / "inventory_report.csv").string();
    ofstream outFile(filePath);

    if (!outFile.is_open()) {
        cout << "Error: Could not create CSV file.\n";
        return;
    }

    outFile << "File Name,Category,Extension,Size in Bytes,Relative Path\n";
    for (const auto& f : allFiles) {
        outFile << "\"" << f.name << "\","
                << "\"" << f.category << "\","
                << "\"" << f.extension << "\","
                << f.sizeInBytes << ","
                << "\"" << f.relativePath << "\"\n";
    }

    outFile.close();
    cout << "\nSuccess: CSV Report saved successfully at:\n" << filePath << "\n";
}

// Module 7: Safe Organization (Copy into Category Folders)
void organizeFiles() {
    fs::path vault = selectedRootFolder / "Organized_Vault";

    cout << "\n================ SAFE ORGANIZATION =================\n";
    cout << "Destination: " << vault.string() << "\n";
    cout << "Are you sure you want to copy files into category folders? (y/n): ";
    char confirm;
    cin >> confirm;
    cin.ignore();

    if (confirm != 'y' && confirm != 'Y') {
        cout << "Action cancelled. No files were changed.\n";
        return;
    }

    int copied = 0;
    for (const auto& f : allFiles) {
        // Source code ya executable files ko move na karein
        if (f.extension == ".cpp" || f.extension == ".exe" || f.extension == ".cbp") continue;

        fs::path targetFolder = vault / f.category;
        fs::create_directories(targetFolder);

        fs::path finalDestination = targetFolder / f.name;

        // Agar file pehle se ho to overwrite nahi karega (Data Safety)
        if (fs::exists(finalDestination)) {
            finalDestination = targetFolder / ("copy_" + f.name);
        }

        fs::copy_file(f.fullPath, finalDestination, fs::copy_options::skip_existing);
        cout << "Copied: " << f.name << " -> " << f.category << "/\n";
        copied++;
    }

    cout << "\nSuccess: " << copied << " files organized safely.\n";
    scanDirectory(selectedRootFolder); // Refresh inventory
}

int main() {
    cout << "====================================================\n";
    cout << "       DIGITAL PHOTO AND FILE ORGANIZER             \n";
    cout << "====================================================\n";

    string folderInput;
    cout << "Enter target folder path (e.g. C:\\TestFolder):\n> ";
    getline(cin, folderInput);

    // Agar double quotes lage ho to hatana
    folderInput.erase(remove(folderInput.begin(), folderInput.end(), '\"'), folderInput.end());

    if (folderInput.empty()) {
        selectedRootFolder = fs::current_path();
    } else {
        selectedRootFolder = fs::u8path(folderInput);
    }

    if (!fs::exists(selectedRootFolder) || !fs::is_directory(selectedRootFolder)) {
        cout << "Folder does not exist! Please check the path.\n";
        return 0;
    }

    cout << "Scanning folder...\n";
    scanDirectory(selectedRootFolder);
    cout << "Done! Found " << allFiles.size() << " files.\n";

    int choice = 0;
    while (true) {
        cout << "\n--- MAIN MENU ---\n";
        cout << "1. View Dashboard & Top 10 Files\n";
        cout << "2. Search Files\n";
        cout << "3. Find Duplicate Files\n";
        cout << "4. Show Visual Graph Summary\n";
        cout << "5. Export Inventory to CSV\n";
        cout << "6. Safely Organize Files into Folders\n";
        cout << "7. Exit\n";
        cout << "Enter your choice (1-7): ";
        cin >> choice;
        cin.ignore();

        if (choice == 1) showDashboard();
        else if (choice == 2) searchFiles();
        else if (choice == 3) findDuplicates();
        else if (choice == 4) showVisualChart();
        else if (choice == 5) exportToCSV();
        else if (choice == 6) organizeFiles();
        else if (choice == 7) {
            cout << "Exiting program. Allah Hafiz!\n";
            break;
        } else {
            cout << "Invalid choice! Enter a number from 1 to 7.\n";
        }
    }

    return 0;
}
