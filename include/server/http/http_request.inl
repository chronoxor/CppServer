/*!
    \file http_request.inl
    \brief HTTP request inline implementation
    \author Ivan Shynkarenka
    \date 07.02.2019
    \copyright MIT License
*/

namespace std {

template <>
struct hash<CppServer::HTTP::HTTPRequest>
{
    typedef CppServer::HTTP::HTTPRequest argument_type;
    typedef size_t result_type;

    result_type operator() (const argument_type& value) const
    {
        result_type result = 17;
        result = result * 31 + std::hash<std::string>()(value.cache());
        return result;
    }
};

} // namespace std
