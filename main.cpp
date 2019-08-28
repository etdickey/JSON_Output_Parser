/**
 * Author: Ethan Dickey
 */
#include "JSON_File.h"
#include <string>

using namespace std;

//Make life easier
#define WRONGFUL_CLOSING_ERROR    JSON_File::WRONGFUL_CLOSING_ERROR
#define OBJECT_IN_ARRAY_ERROR     JSON_File::OBJECT_IN_ARRAY_ERROR
#define NOT_INITIALIZED_ERROR     JSON_File::NOT_INITIALIZED_ERROR
//Exit the program whenever a test fails or just continue for fun
#define EXIT_ON_FAIL        1

//Vectors to use in testing
vector<string> myNames({"my", "name", "is", "hard"});
vector<int> myInts({1, 2, 5, 6});
vector<double> myDoubles({1.1, 2.2, 4.4, 7.7});
vector<bool> myTruths({true, true, false, false});

/**
 *  A series of tests for the JSON_File class.  Some require visual observation for confirmation.
 */
//Basic test
void sanityCheck(JSON_File& json);
//Illustrate a bug in the code: multi-typed array
void multiTypedArray(JSON_File& json);
//Illustrate a few bugs with bools not printing correctly
void boolBug(JSON_File& json);

//Test 2d arrays and describe proper sub array syntax
void twoDArrays(JSON_File& json);
//Test incorrectly closing objects/arrays (@RETURN SUCCESS)
bool incorrectClosing(JSON_File& json);
//Test close_until
bool testCloseUntil(JSON_File& json);
//Test closing brackets upon deconstructing
bool finalTestDestructor(JSON_File& json);

//Test initialization guard (file MUST not be initailized)
bool unitializedTest(JSON_File& json, string& message);
//Test chaining commands
void chainingTest(JSON_File& json);
// //Test the safeguard for negative tabs //unnecessary
// void negativeTabTest(JSON_File& json);
//Test passing in a vector and the function detecting what type it is
void vectorTest(JSON_File& json);



int main(){
    //initialize json object
    JSON_File json("out.json");

    try {
        //Begin testing
        sanityCheck(json);

        //Illustrate a bug in the code
        multiTypedArray(json);

        //Illustrate a few bugs with bools not printing correctly
        boolBug(json);

        //Test 2d arrays and describe proper sub array syntax
        twoDArrays(json);

        //Test incorrectly closing objects/arrays
        if(!incorrectClosing(json)){
            cerr << "FAILED TO HANDLE INCORRECT CLOSING: EXIT()" << endl;
            #if EXIT_ON_FAIL
                exit(1);
            #endif
        }

        //Test close_until
        if(!testCloseUntil(json)){
            cerr << "close_until FAILED: EXIT()" << endl;
            #if EXIT_ON_FAIL
                exit(1);
            #endif
        }

        //Test closing brackets upon deconstructing
        if(!finalTestDestructor(json)){
            cerr << "FAILED TO PREVENT USER FROM PRINTING A NAMED ARRAY IN ARRAY" << endl;
            #if EXIT_ON_FAIL
                exit(1);
            #endif
        }
    } catch(NOT_INITIALIZED_ERROR* e){
        cerr << "Err: not initialized (message = " << e->what() << ")" << endl;
    }


    //Begin round 2
    JSON_File json2;
    string message;

    //Test unitialized
    if(!unitializedTest(json2, message)){
        cerr << message << endl;
        #if EXIT_ON_FAIL
            exit(1);
        #endif
    }

    json2.open("secondFile.out");
    //test chaining of functions
    chainingTest(json2);

    //Test using the initializer list function for print_data
    vectorTest(json2);

    json2.close();

    return 0;
}

/**
 * Basic tests and demonstrations
 */
//basic test
void sanityCheck(JSON_File& json){
    json.open_object("myObject");
    json.open_array("myArray1!!");
    json.print_data(myNames);
    json.close_array();

    json.print_array("my ints?", myInts);
    json.print_array("my truths ;)", myTruths);
    json.close_object();
}

//Illustrate a bug in the code: multi-typed array
void multiTypedArray(JSON_File& json){
    json.open_object("New object!!!");
    json.open_array("Many-typed array ohno");
    json.print_data(myInts);
    json.print_data(myTruths);
    json.print_data(myNames);
    json.print_data(myDoubles);
    json.close_array();
    json.close_object();
}

//Illustrate a few bugs with bools not printing correctly
void boolBug(JSON_File& json){
    json.open_object("I've gotta get back to work");
    json.print_element("name", "ions");
    json.print_element("note: ", "[SEE BELOW] We currently don't handle bools correctly, they're supposed to be true or false, oops");
    json.print_element("note: ", "I FIXED IT WOOOOO^^");
    json.print_element("an int?", 2);
    json.print_element("a double?", 4000.89);
    json.print_element("a bool?", true);
    json.print_element("LOOK:", "oh hey watch me lie to you about the type :o");
    json.print_element<bool>("a bool (heh, bug will be fixed eventually [NOW FIXED!!])?", 1);
    json.print_element("an int?", 1);
    json.close_object();
}

/**
 * More detailed tests and demonstrations
 */
//Test 2d arrays and describe proper sub array syntax
void twoDArrays(JSON_File& json){
    json.open_object("Moar testing");
    json.open_array("Testing 2d and 3d arrays");
    json.open_sub_array();
    json.print_data(myInts);
    json.close_sub_array();
    json.close_array();

    json.print_array("IMPORTANT NOTE", vector<string>({"If you use open_sub_array(), print_data(),",
                             "close_sub_array() instead of using print_sub_array,",
                             "you will get extra tabs that don't look pretty"}));

    json.open_array("An example follows [BAD] (3d -- 1x1xDIM)");
    int someArbitraryNumber = 5;
    json.open_sub_array();
    for(int i=0;i<someArbitraryNumber;i++){
        json.open_sub_array();
        json.print_data(myInts);
        json.close_sub_array();
    }
    json.close_sub_array();
    json.close_array();

    json.open_array("This is much better [GOOD] (3d -- 1x1xDIM)");
    json.open_sub_array();
    for(int i=0;i<someArbitraryNumber;i++){
      json.print_sub_array(myInts);
    }
    json.close_sub_array();
    json.close_array();
    json.close_object();
}

//Test incorrectly closing objects/arrays (@RETURN SUCCESS)
bool incorrectClosing(JSON_File& json){
    json.open_object("Testing throwing clause");
    json.open_array("Array trying incorrect [closing object]");
    json.print_data(myInts);
    try {
        json.close_object();
        json.print_element("FAILURE", "SHOULD HAVE ERRORED");

        return false;
    } catch(WRONGFUL_CLOSING_ERROR* e){
        json.close_array();
        json.print_element("Success", "Program successfully failed when it was supposed to");
        json.print_element("Message:", e->what());
    }

    json.open_array("Sub array trying incorrect [closing object] and with bad indentation in order to unit test");
    json.open_sub_array();
    json.print_data(myDoubles);
    json.close_sub_array();
    json.open_sub_array();
    json.print_data(myDoubles);
    try {
        json.close_object();
        json.print_element("FAILURE", "SHOULD HAVE ERRORED");

        return false;
    } catch(WRONGFUL_CLOSING_ERROR* e){
        json.close_sub_array();
        json.close_array();
        json.print_element("Success", "Program successfully failed when it was supposed to:");
        json.print_element("Message:", e->what());
    }

    try {
        json.close_array();
        json.print_element("FAILURE", "SHOULD HAVE ERRORED");

        return false;
    } catch(WRONGFUL_CLOSING_ERROR* e){
        json.close_object();
        json.print_element("Success", "Program successfully failed when it was supposed to");
        json.print_element("Message:", e->what());
    }

    return true;
}

//Test close_until
bool testCloseUntil(JSON_File& json){
    json.open_object("Testing close_until [level 1]");
    json.open_object("Level 2");
    json.open_object("Level 3");
    json.open_object("Level 4");
    json.print_element("Man", "This is a lot of levels");
    json.open_object("Level 5");
    json.open_object("Level 6");

    int backToArr[] = {4, 3, 1, 0};//must be decreasing
    for(int i=0;i<4;i++){
        //announce
        json.print_element("Take me back to level", backToArr[i]);
        if(i==0){
            json.open_array("Level 7: empty array");
        }

        //close
        json.close_until(backToArr[i]);
        string output = "Level ";
        output += (backToArr[i] + '0');
        output += "?";
        json.print_element(output, (json.getCurrentLevel() == backToArr[i]));

        //check
        if(json.getCurrentLevel() != backToArr[i]){ return false; }
    }

    json.print_element("Take me back to level (invalid)", -1);
    json.close_until(-1);
    json.print_element("Level -1 (should fail))?", (json.getCurrentLevel() == -1));
    if(json.getCurrentLevel() == -1){ return false; }

    json.print_element("Take me back to level (bigger than current stack test)", 4);
    json.close_until(4);
    json.print_element("Level 4 (should fail)?", (json.getCurrentLevel() == 4));
    if(json.getCurrentLevel() == 4){ return false; }

    return true;
}

//Test closing brackets upon deconstructing
bool finalTestDestructor(JSON_File& json){
    json.open_object("Another test! ([this MUST be the last test to test the deconstructor])");
    json.print_element("Idea1", "The idea for this one is to test the deconstructor in putting the brackets in the right place");
    json.print_element("Idea2", "because it should be able to finish off any brackets that aren't in place");
    json.open_object("When it deconstructs");
    json.open_array("An array of objects (or named arrays) is not allowed");
    try{
        json.print_array("So we make try", vector<string>({"Oo there's a string here", "Yeah right here, not that exciting"}));
        return false;
    } catch(OBJECT_IN_ARRAY_ERROR* e){
        json.print_sub_array(vector<string>({"Success", "Program successfully failed when it was supposed to"}));
        json.print_sub_array(vector<string>({"Message:", e->what()}));
    }
    json.close_array();

    json.open_array("Another");
    json.print_sub_array(vector<string>({"Oo there's a string here", "Yeah right here, not that exciting"}));
    json.print_sub_array(vector<string>({"String arrays :)"}));
    json.close_array();

    json.open_array("LAST ONE");
    json.print_sub_array(vector<string>({"After this the program should automatically inject the proper closing brackets and braces"}));

    return true;
}

/**
 * Unique tests
 */
//File MUST be unitialized
bool unitializedTest(JSON_File& json, string& message){
    string m1 = "ERROR: SUCCESSFULLY CALLED ", m2 = " ON UNITIALIZED JSON_File";

    if(json.isInitialized()){
        message = "ERROR: CALLED UNITIALIZEDTEST ON AN INITAILIZED OBJECT";
        return false;
    }

    //close
    try{
        json.close();
        message = m1 + ".close()" + m2;
        return false;
    } catch(NOT_INITIALIZED_ERROR* e){ /*success*/ }

    //open and close object
    try{
        json.open_object("temp");
        message = m1 + ".open_object(\"temp\")" + m2;
        return false;
    } catch(NOT_INITIALIZED_ERROR* e){ /*success*/ }
    try{
        json.close_object();
        message = m1 + ".close_object()" + m2;
        return false;
    } catch(NOT_INITIALIZED_ERROR* e){ /*success*/ }

    //open and close array
    try{
        json.open_array("temp");
        message = m1 + ".open_array(\"temp\")" + m2;
        return false;
    } catch(NOT_INITIALIZED_ERROR* e){ /*success*/ }
    try{
        json.close_array();
        message = m1 + ".close_array()" + m2;
        return false;
    } catch(NOT_INITIALIZED_ERROR* e){ /*success*/ }

    //open and close sub array
    try{
        json.open_sub_array();
        message = m1 + ".open_sub_array()" + m2;
        return false;
    } catch(NOT_INITIALIZED_ERROR* e){ /*success*/ }
    try{
        json.close_sub_array();
        message = m1 + ".close_sub_array()" + m2;
        return false;
    } catch(NOT_INITIALIZED_ERROR* e){ /*success*/ }

    //print data
    try{
        json.print_data(myInts);
        message = m1 + ".print_data(myInts)" + m2;
        return false;
    } catch(NOT_INITIALIZED_ERROR* e){ /*success*/ }
    try{
        json.print_array("temp", myInts);
        message = m1 + ".print_array(\"temp\", myInts)" + m2;
        return false;
    } catch(NOT_INITIALIZED_ERROR* e){ /*success*/ }
    try{
        json.print_sub_array(myInts);
        message = m1 + ".print_sub_array(myInts)" + m2;
        return false;
    } catch(NOT_INITIALIZED_ERROR* e){ /*success*/ }
    try{
        json.print_element("temp", 2);
        message = m1 + ".print_element(\"temp\", 2)" + m2;
        return false;
    } catch(NOT_INITIALIZED_ERROR* e){ /*success*/ }

    //close until
    try{
        json.close_until(2);
        message = m1 + ".close_until(2)" + m2;
        return false;
    } catch(NOT_INITIALIZED_ERROR* e){ /*success*/ }


    return true;
}

//Test chaining commands
void chainingTest(JSON_File& json){
    json.open_object("testing chaining!").open_object("test1...").open_array("test2! [empty]").close_array();

    json.open_array("test3 -- bad practice").open_sub_array().print_data(myInts).close_until(2);
    json.open_array("test4 -- better practice").print_sub_array(myInts).close_array();

    json.print_array("test5 :)", myTruths)
        .print_array("test6 :)", myDoubles)
        .print_array("test7 :)", myNames)
        .print_element("test8  :)", "S")
        .print_element("test9  :)", "i")
        .print_element("test10 :)", "c")
        .print_element("test11 :)", " ")
        .print_element("test12 :)", "\'")
        .print_element("test13 :)", "e")
        .print_element("test14 :)", "m")
        .print_element("test15 :)", " ")
        .print_element("test16 :)", "B")
        .print_element("test17 :)", "E")
        .print_element("test18 :)", "A")
        .print_element("test19 :)", "R")
        .print_element("test20 :)", "S")
        .print_element("test21 :)", "!!!!!!!!!!!!!!!!!!").close_object();
    json.close_until(0);
}

//Test passing in a vector and the function detecting what type it is
void vectorTest(JSON_File& json){
    json.print_element("TESTING INITIALIZER LISTS", "Try .print_[...]({val, val, val}) instead of passing in vectors");

    json.print_array(".print_array({1, 2, 3})", {1, 2, 3});
    json.open_array(".print_data({1, 2, 3})").print_data({1, 2, 3}).close_array();
    // auto& a = [1, 2, 3];
    // auto& b = [4, 5, 6];
    // auto& c = [7, 8, 9];
    // int a[] = {1, 2, 3};
    // int b[] = {4, 5, 6};
    // int c[] = {7, 8, 9};
    // json.print_array(".print_array({myInts, myInts, myInts})", {a, b, c});
}
