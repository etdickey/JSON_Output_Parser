#!/bin/bash

g++ -std=c++11 main.cpp -o ./a.out
if [ "$1" = "runcode" ]; then
  ./a.out
  cat out.json
  
  echo -e "\n\nSecond set of tests"
  
  cat secondFile.out.json
  
  echo -e "Validating json"
  
  if [ "$2" = "print" ]; then
    python -mjson.tool out.json   
  
    
    python -mjson.tool secondFile.out.json 
  fi
  
  echo JSON Valid!
fi
