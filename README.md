### DCompute

 These are the release notes for DCompute 1.0.That will tell you all about, explain how to deploy it.

 that work flow is this:

 1.All computer in one LAN can construct a distributed computing system with DCompute now.So first be sure all computer in one LAN.

 2.Copy the controller.exe bin file and the computing library to one computer.the computing library is one dll file,that include some function for computing, that hava standard prototype. like the example library math.dll in this project. you will to costomize yourself computing library to do the computing you want.

 3.Copy the computing_server.exe bin and the computing library also to some other computers.

 4.Copy the bp_client.exe to one computer, and write one command script file,that format is:

    computing_library_file_name:computing_function_name{\\[IUDF]:value}*

    this "\\" delimiter betwwen two parameter groups or betwwen parameter group and command.You input parameter group time ( delim it by "\\" ) is the execution time of the computing process.

 5.Start controller.exe and computing_server.exe server side program. now execute the bp_client.exe with the file name of the command script.( is parameter 1 ) to be starting the distributed computing. 

 6.Some information and result will be showing to cosole screen.

 About us:
 
 This project is developing only 2 weeks, that will need to update often, but I'm busy in my work, if you is one c programmer, and intrest with this project, please join me.send your information to my email.

 Note: This project is alpha version now, do not promise stable. if you find bug or you want to join us,please contact me:

 Email: simple_ai@outlook.com

 Thank you.

 JiJie Shi 2009
