/*!
    \file http_response.inl
    \brief HTTP response inline implementation
    \author Ivan Shynkarenka
    \date 15.02.2019
    \copyright MIT License
*/

#if defined(FMT_VERSION)
template <> struct fmt::formatter<CppServer::HTTP::HTTPResponse> : ostream_formatter {};
#endif

template <>
struct std::hash<CppServer::HTTP::HTTPResponse>
{
    typedef CppServer::HTTP::HTTPResponse argument_type;
    typedef size_t result_type;

    result_type operator() (const argument_type& value) const
    {
        result_type result = 17;
        result = result * 31 + std::hash<std::string>()(value.cache());
        return result;
    }
};
