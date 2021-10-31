#ifndef SHIKEN_DOCUMENT_PARSER_H
#define SHIKEN_DOCUMENT_PARSER_H

#include <shiken/config.h>
#include <xsde/cxx/parser/elements.hxx>

namespace shiken {

    /**
     * This template class can be used to transform any XSD/e parser into a 
     * parser that overrides namespaces of the XML being parsed.
     * 
     * This may be necessary when parsing XML files that do not define any
     * namespaces.
     * 
     * \tparam Base                     Base class, a parser to extend.
     */
    template<class Base>
    class NamespaceOverrideParser: public Base {
    public:
        NamespaceOverrideParser(): m_namespaceOverride(NULL) {}

        const char *namespaceOverride() const {
            return m_namespaceOverride.data();
        }

        /**
         * \param namespaceOverride     Name of the namespace to override XML namespaces with. 
         *                              Pass NULL to disable namespace overriding.
         */
        void setNamespaceOverride(const char *namespaceOverride) {
            m_namespaceOverride.assign(namespaceOverride);
        }

    protected:
        virtual void _start_element(const xml_schema::ro_string &ns, const xml_schema::ro_string &name) override {
            if(m_namespaceOverride.data() != NULL) {
                Base::_start_element(m_namespaceOverride, name);
            } else {
                Base::_start_element(ns, name);
            }
        }

        virtual void _attribute(const xml_schema::ro_string &ns, const xml_schema::ro_string &name, const xml_schema::ro_string &value) override {
            if(m_namespaceOverride.data() != NULL) {
                Base::_attribute(m_namespaceOverride, name, value);
            } else {
                Base::_attribute(ns, name, value);
            }
        }

    private:
        xml_schema::ro_string m_namespaceOverride;
    };

} // namespace shiken

#endif // SHIKEN_DOCUMENT_PARSER_H
