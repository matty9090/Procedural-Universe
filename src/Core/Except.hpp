#include <exception>
#include <stdexcept>
#include <string>
#include <stdio.h>

class file_not_found : public std::exception
{
public:
    file_not_found(std::string f) : file(f) {}

    virtual const char* what() const override
    {
        return (std::string("File not found: ") + file).c_str();
    }

private:
    std::string file;
};

namespace DX
{
    // Helper class for COM exceptions
    class com_exception : public std::exception
    {
    public:
        com_exception(HRESULT hr) : result(hr) {}

        virtual const char* what() const override
        {
            static char s_str[64] = {};
            sprintf_s(s_str, "Failure with HRESULT of %08X", static_cast<unsigned int>(result));
            return s_str;
        }

    private:
        HRESULT result;
    };

    // Helper utility converts D3D API failures into exceptions.
    inline void ThrowIfFailed(HRESULT hr)
    {
        if (FAILED(hr))
        {
            throw com_exception(hr);
        }
    }
}
