@echo Counting code...

@"./code_statistic.py" ../../eflib .h;.cpp;.py;.xgt;.xgp;.txt;.cmake > codelines_of_salvia.txt
@"./code_statistic.py" ../../salviar .h;.cpp;.py;.xgt;.xgp;.txt;.cmake >> codelines_of_salvia.txt
@"./code_statistic.py" ../../salviax .h;.cpp;.py;.xgt;.xgp;.txt;.cmake >> codelines_of_salvia.txt
@"./code_statistic.py" ../../samples .h;.cpp;.py;.xgt;.xgp;.txt;.cmake >> codelines_of_salvia.txt
@"./code_statistic.py" ../../sasl .h;.cpp;.py;.xgt;.xgp;.txt;.cmake >> codelines_of_salvia.txt

@echo Code counting finished.
@echo off
pause