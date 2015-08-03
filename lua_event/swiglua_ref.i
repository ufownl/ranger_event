%include <lua_fnptr.i>

%{
class swiglua_ref {
public:
    swiglua_ref(const SWIGLUA_REF& ref)
        : m_ref(ref) {
    }

    ~swiglua_ref() {
        swiglua_ref_clear(&m_ref);
    }

    void get() {
        swiglua_ref_get(&m_ref);
    }

    lua_State* L() const {
        return m_ref.L;
    }

private:
    SWIGLUA_REF m_ref;
};
%}
