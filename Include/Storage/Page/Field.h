#ifndef FIELD_H
#define FIELD_H

#include <iostream>
#include <variant>
#include <cstring>
#include <iomanip>
using namespace std;
enum FieldType { TYPE_INT, TYPE_STRING, TYPE_FLOAT, TYPE_BOOL, TYPE_NULL};

inline string getFieldTypeasString(FieldType type){
    switch (type)
    {
    case TYPE_INT:return "TYPE_INT";
    case TYPE_STRING:return "TYPE_STRING";
    case TYPE_FLOAT:return "TYPE_FLOAT";
    case TYPE_BOOL:return "TYPE_BOOL";
    default:
        break;
    }
    return "NULL";
}

using FieldValue = variant<int, string, double, bool>;

class Field {
private:
    FieldType fieldType;
    int size;
    bool is_null{false};
    
    union {
        int VALUE_INT;
        const char* VALUE_STRING;
        double VALUE_FLOAT;
        bool VALUE_BOOL;
    };

public:
    Field(FieldType type, int value);
    Field(FieldType type, double value);
    Field(FieldType type, bool value);
    Field(FieldType type, const char* value);
    Field(const Field& other);
    Field(FieldType type);
    Field(){}
    ~Field();

    void setValue(int value);
    void setValue(double value);
    void setValue(bool value);
    void setValue(const char* value);


    int getFieldValueInt()const;
    float getFieldValueFloat()const;
    bool getFieldValueBool()const;
    const char* getFieldValueStr()const;

    FieldValue getFieldValue()const;
    FieldType getFieldType()const;

    bool isNull() ;
    void set_null(bool value){
        this->is_null = value;
    }
    int getSize() ;
    int getSerializedSize() const ;

    void print();
    void serialize(char* buffer);
    void deserialize(char* buffer);


    Field& operator=(const Field& other);

    bool operator==(const Field& other)const;

    bool operator>(const Field& other)const;
    bool operator<(const Field& other)const;
    bool operator>=(const Field& other)const;
    bool operator<=(const Field& other)const;


};


namespace std {

template<>
struct hash<Field> {

    size_t operator()(const Field& f) const {

        switch (f.getFieldType()) {

            case TYPE_INT:
                return hash<int>()(
                    f.getFieldValueInt()
                );

            case TYPE_FLOAT:
                return hash<float>()(
                    f.getFieldValueFloat()
                );

            case TYPE_BOOL:
                return hash<bool>()(
                    f.getFieldValueBool()
                );

            case TYPE_STRING:
                return hash<string>()(
                    string(f.getFieldValueStr())
                );

            default:
                return 0;
        }
    }
};

}


#endif 