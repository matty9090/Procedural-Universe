#pragma once

#include <exception>
#include <stdexcept>
#include <string>
#include <stdio.h>
#include <comdef.h>
#include <winerror.h>

namespace DX
{
    // Helper class for COM exceptions
    class com_exception : public std::exception
    {
    public:
        com_exception(HRESULT hr) : result(hr) {}

        virtual const char* what() const override
        {
            _com_error err(result);
            LPCTSTR errMsg = err.ErrorMessage();

            TCHAR szBuffer[512];

            FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,
                NULL, result,
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                (LPTSTR)szBuffer, 512, NULL);

            static char s_str[512] = {};
            sprintf_s(s_str, "Failure with HRESULT of %08X (%ls)", static_cast<unsigned int>(result), szBuffer);
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
