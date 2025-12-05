
#include <Se/Console.hpp>
#include <Se/IO/Package.h>

namespace Se
{

void Package::Scan(std::vector<String>& result, const String& pathName, const String& filter, ScanFlags flags) const
{
    result.clear();

    String sanitizedPath = GetSanitizedPath(pathName);
    String filterExtension;
    std::size_t dotPos = filter.rfind('.');
    if (dotPos != std::string::npos)
        filterExtension = filter.substr(dotPos);
    if (filterExtension.contains('*'))
        filterExtension.clear();

    bool caseSensitive = true;
#ifdef _WIN32
    // On Windows ignore case in string comparisons
    caseSensitive = false;
#endif

    auto entryNames = GetEntryNames();
    for (auto i = entryNames.begin(); i != entryNames.end(); ++i)
    {
        String entryName = GetSanitizedPath(*i);
        if ((filterExtension.empty() || entryName.ends_with(filterExtension, caseSensitive)) &&
            entryName.starts_with(sanitizedPath, caseSensitive))
        {
            String fileName = entryName.substr(sanitizedPath.length());
            if (fileName.starts_with("\\") || fileName.starts_with("/"))
                fileName = fileName.substr(1, fileName.length() - 1);
            if (!flags.Test(ScanFlag::SCAN_RECURSIVE) && (fileName.contains("\\") || fileName.contains("/")))
                continue;

            result.push_back(fileName);
        }
    }
}

void Package::ScanTree(DirectoryNode& result, const String& pathName, const String& filter, ScanFlags flags) const
{
    result.Children.clear();

    auto entries = GetEntryNames();
    for (auto entry : entries) {
        TreeNodeAddPath(&result, entry);
    }
}

}