/**
 * Author: Ethan Dickey
 */
#include <iostream>
#include <string>
#include <fstream>
#include <stack>
#include <vector>
#include <typeinfo>

#define endl '\n'

using namespace std;

class JSON_File {
private:
    bool comma, initialized;
    ofstream out;//output stream
    stack<char> brackets;//keeps track in case of mass closing and also as a safeguard for wrongful closing (object for array, etc.)
    int currDepth;//increments by 2 -- the true padding number in spaces (as opposed to true depth, which is half)
    int lowestArrayDepth;

    template <class T>
    void print_data(vector<T> data, bool tabs);
    void print_type(string val) { out << "\"" << val << "\""; }
    void print_type(const char* val) { out << "\"" << val << "\""; }
    void print_type(bool val) { out << (val == true ? "true" : "false"); }
    void print_type(double val) { out << val; }
    void print_type(int val) { out << val; }
    // template <class T>
    // void print_type(T val) { out << val; }

public:

    /**
     * The following are objects to be thrown by member functions when needed
     */
    struct WRONGFUL_CLOSING_ERROR : public exception {
        char correct_brace;
        string message;

        WRONGFUL_CLOSING_ERROR(char cb, string m): correct_brace(cb), message(m) {}
        const char* what() const throw(){ return message.c_str(); } //for c++ //(string)("Correct brace: " + correct_brace +
    };
    struct OBJECT_IN_ARRAY_ERROR : public exception {
        string message;

        OBJECT_IN_ARRAY_ERROR(string m): message(m) {}
        const char* what() const throw(){ return message.c_str(); }//for c++
    };
    struct NOT_INITIALIZED_ERROR : public exception {
        string message;

        NOT_INITIALIZED_ERROR(string m): message(m) {}
        const char* what() const throw(){ return message.c_str(); }//for c++
    };

    JSON_File(): comma(false), initialized(false), currDepth(-1), lowestArrayDepth(-1) {}
    JSON_File(string filename): initialized(false) {//redundant safeguard with initialization
        this->open(filename);
    }

    ~JSON_File() {
        if(this->initialized){
            this->close();
        }
    };

    /**
     * Description: opens the file
     *
     * @param  filename : the file name
     * @return bool     : success or failure
     */
    bool open(string filename);
    /**
     * Description: closes the file
     *
     * @return void
     */
    void close();

    JSON_File& open_object(string name);
    void close_object();

    JSON_File& open_array(string name);
    void close_array();

    JSON_File& open_sub_array();
    void close_sub_array();

    template <class T>
    JSON_File& print_data(vector<T> data);
    template <class T>
    JSON_File& print_data(std::initializer_list<T> t){ return print_data(vector<T>(t)); }

    //Simple print an array with a name
    template <class T>
    JSON_File& print_array(string name, vector<T> data);
    template <class T>
    void print_array(string name, std::initializer_list<T> data){ print_array(name, vector<T>(data)); }

    //Print a sub array (another dimension) with no name and inline
    template <class T>
    JSON_File& print_sub_array(vector<T> data);//, bool withNewLine = false);

    template <class T>
    JSON_File& print_element(string name, T val);

    void close_until(int levelNonInclusive);
    int getCurrentLevel(){ return brackets.size();}
    bool isInitialized(){ return initialized; }
};


/**
 * Implementations of JSON_File
 *
 */

/**
 * Open/close file functions
 */
bool JSON_File::open(string filename){
    if(!initialized){
        //Initialize
        comma = false;
        currDepth = 2;
        lowestArrayDepth = -1;
        initialized = false;

        //Append .json to the end of the file
        if(filename.length() < 5 || filename.substr(filename.length()-5, 5) != ".json"){
            filename += ".json";
        }

        //Open the file
        out.open(filename.c_str());
        if(out.is_open()){
            out << "{\n";

            initialized = true;
        }
    } else {
        throw new NOT_INITIALIZED_ERROR("CALLED JSON_File::open() FILE ALREADY OPEN");
    }

    //Return success?
    return initialized;
}
void JSON_File::close(){
    if(initialized){
        //Close all preceeding brackets
        close_until(0);

        //Safeguard
        initialized = false;

        //Print the last closing bracket
        out << "\n}\n";

        //Close the file
        out.close();

        //Clean up
        comma = false;
        currDepth = -1;
        lowestArrayDepth = -1;
    } else {
        throw new NOT_INITIALIZED_ERROR("CALLED JSON_File::close() WITHOUT INITIALIZING");
    }
}

/**
 * Open/close object functions
 */
JSON_File& JSON_File::open_object(string name){
    if(initialized){
        if(lowestArrayDepth != -1){
            throw new OBJECT_IN_ARRAY_ERROR("DON'T PUT AN OBJECT IN AN ARRAY JSON_File::open_object:");
        }

        string depth(currDepth, ' ');

        if(comma) out << ",\n";

        out << depth << "\"" << name << "\": {\n";

        currDepth += 2;
        comma = false;
        brackets.push('}');
    } else {
        throw new NOT_INITIALIZED_ERROR("CALLED JSON_File::open_object() WITHOUT INITIALIZING");
    }

    return *this;
}
void JSON_File::close_object(){
    if(initialized){
        if(brackets.top() != '}'){ throw new WRONGFUL_CLOSING_ERROR('}', "Need '}' in JSON_File::close_object ");}

        currDepth -= 2;

        string depth(currDepth, ' ');
        out << '\n' << depth << "}";

        comma = true;
        brackets.pop();
    } else {
        throw new NOT_INITIALIZED_ERROR("CALLED JSON_File::close_object() WITHOUT INITIALIZING");
    }
}

JSON_File& JSON_File::open_array(string name){
    if(initialized){
        if(lowestArrayDepth != -1){
            throw new OBJECT_IN_ARRAY_ERROR(("DON'T PUT A NAMED ARRAY IN AN ARRAY (use subarray) JSON_File::open_array:"));
        }
        string depth(currDepth, ' ');

        if(comma) out << ",\n";
        out << depth << "\"" << name << "\": [\n";

        currDepth += 2;
        comma = false;
        brackets.push(']');
        if(lowestArrayDepth == -1){ lowestArrayDepth = brackets.size(); }
    } else {
        throw new NOT_INITIALIZED_ERROR("CALLED JSON_File::open_array() WITHOUT INITIALIZING");
    }

    return *this;//chaining
}
void JSON_File::close_array(){
    if(initialized){
        if(brackets.top() != ']'){ throw new WRONGFUL_CLOSING_ERROR(']', "Need ']' in JSON_File::close_array ");}
        currDepth -= 2;

        string depth(currDepth, ' ');
        out << '\n' << depth << "]";

        if(lowestArrayDepth == brackets.size()){ lowestArrayDepth = -1; }
        comma = true;
        brackets.pop();
    } else {
        throw new NOT_INITIALIZED_ERROR("CALLED JSON_File::close_array() WITHOUT INITIALIZING");
    }
}

JSON_File& JSON_File::open_sub_array(){
    if(initialized){
        string depth(currDepth, ' ');

        if(comma) out << ", ";
        else out << depth;

        out << "[";
        comma = false;
        brackets.push(']');
        if(lowestArrayDepth == -1){ lowestArrayDepth = brackets.size(); }
    } else {
        throw new NOT_INITIALIZED_ERROR("CALLED JSON_File::open_sub_array() WITHOUT INITIALIZING");
    }

    return *this;//chaining
}
void JSON_File::close_sub_array(){
    if(initialized){
        if(brackets.top() != ']'){ throw new WRONGFUL_CLOSING_ERROR(']', "Need ']' in JSON_File::close_sub_array ");}
        out << "]";

        if(lowestArrayDepth == brackets.size()){ lowestArrayDepth = -1; }
        comma = true;
        brackets.pop();
    } else {
        throw new NOT_INITIALIZED_ERROR("CALLED JSON_File::close_sub_array() WITHOUT INITIALIZING");
    }
}

template <class T>
JSON_File& JSON_File::print_data(vector<T> data){//<vector<T>>
    if(initialized){
        print_data(data, true);
    } else {
        throw new NOT_INITIALIZED_ERROR("CALLED JSON_File::print_data(public function) WITHOUT INITIALIZING");
    }

    return *this;//chaining
}

// void JSON_File::print_type(string val) { out << "\"" << data[i] << "\""; }
// void JSON_File::print_type(bool val) { out << (data[i] == true ? "True" : "False"); }
// void JSON_File::print_type(double val) { out << data[i]; }
// void JSON_File::print_type(int val) { out << data[i]; }

template <class T>
void JSON_File::print_data(vector<T> data, bool tabs){
    if(initialized){
        //tabs and newline
        string depth(currDepth, ' ');
        if(comma) out << ", ";
        else if(tabs) out << depth;

        for(int i=0;i<data.size();i++){
            out << (i == 0 ? "": ", ");

            print_type(data[i]);
        }
        comma = true;
    } else {
        throw new NOT_INITIALIZED_ERROR("CALLED JSON_File::print_data(private function) WITHOUT INITIALIZING");
    }
}

//Simple print an array with a name
template <class T>
JSON_File& JSON_File::print_array(string name, vector<T> data){
    if(initialized){
        open_array(name);
        print_data(data, true);
        close_array();
        comma = true;
    } else {
        throw new NOT_INITIALIZED_ERROR("CALLED JSON_File::print_array() WITHOUT INITIALIZING");
    }

    return *this;//chaining
}
//Print a sub array (another dimension) with no name and inline
template <class T>
JSON_File& JSON_File::print_sub_array(vector<T> data){//, bool withNewLine){// = false
    if(initialized){
        open_sub_array();
        print_data(data, false);
        close_sub_array();
    } else {
        throw new NOT_INITIALIZED_ERROR("CALLED JSON_File::print_sub_array() WITHOUT INITIALIZING");
    }

    return *this;//chaining
}

template <class T>
JSON_File& JSON_File::print_element(string name, T val){
    if(initialized){
        //tabs and newline
        string depth(currDepth, ' ');
        if(comma) out << ",\n";
        //name
        out << depth << "\"" << name << "\": ";

        //This auto selects the correct overloaded function for the job at runtime with templated parameters :)
        print_type(val);

        comma = true;
    } else {
        throw new NOT_INITIALIZED_ERROR("CALLED JSON_File::print_element() WITHOUT INITIALIZING");
    }

    return *this;
}

void JSON_File::close_until(int levelNonInclusive){
    if(initialized){
        if(0 <= levelNonInclusive && levelNonInclusive < brackets.size()){
            string depth;
            while(levelNonInclusive < brackets.size()){//what if we're inside a sub_array
                if(brackets.top() == '}' || lowestArrayDepth == brackets.size()){//if it's not a sub-array
                    currDepth -= 2;
                    depth.resize(currDepth, ' ');
                    out << '\n' << depth;
                }

                out << brackets.top();

                if(lowestArrayDepth == brackets.size()){ lowestArrayDepth = -1; }
                brackets.pop();
            }
            comma = true;
        }
    } else {
        throw new NOT_INITIALIZED_ERROR("CALLED JSON_File::close_until() WITHOUT INITIALIZING");
    }
}
