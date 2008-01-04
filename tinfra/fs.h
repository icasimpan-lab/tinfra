#include <string>
#include <vector>

namespace tinfra {
namespace fs {
    
void list_files(const char* name, std::vector<std::string>& result);
    
    
void copy(const char* src, const char* dest);
    
void mkdir(const char* name, bool create_parents = true);
    
void rm(const char* name);
void rmdir(const char* name);

void recursive_copy(const char* src, const char* dest);
void recursive_rm(const char* src);

// std::string inliners
inline
void list_files(std::string const& name, std::vector<std::string>& result) { return list_files(name.c_str(), result); }

inline 
void recursive_copy(std::string const& src, std::string const& dest) { return recursive_copy(src.c_str(), dest.c_str()); }

} }

