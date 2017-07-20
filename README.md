# autoupdate
自动更新

开发工具：
Qt:4.8.6
Qt Creator:2.5.2
mingw:4.8.2
C++ Version:C++11

配置文件(config.ini)说明：  
[update]  
update_dir=..  
update_dir表示更新目录，两个点表示上层目录，也可以修改，但目录必须和autoupdate与华码软件的层级关系对应；  

not_update_dirs="id_logs;"  
not_update_dirs表示不更新的目录，因为软件可能产生一些log文件和不重要的文件，这一部分目录无需更新，所以配置一下以便生成的project.manifest不带这一部分信息。目前配置为id_logs，以后可以继续增加不更新的目录，多个目录以分号分隔，用双引号括起来；  

not_update_files="cardreadlog.txt;CollectConfig.ini;IDInfoLog.txt;Log.dat;upload.txt;"  
not_update_files表示不更新的文件，同样是log文件不需要更新，所以要过滤掉，以便project.manifest不带这些信息，以后可以继续增加不更新的文件，多个文件以分号分隔，用双引号括起来；  

update_url=http://127.0.0.1:8080  
update_url表示后台更新地址，需要根据下载地址选择目录，这里举个例子；若下载地址为：  
http://127.0.0.1:8080/project.manifest，则update_url配置成：  
http://127.0.0.1:8080；  
若下载地址为：  
http://127.0.0.1:8080/aa/bb/cc/project.manifest，则update_url配置成：http://127.0.0.1:8080/aa/bb/cc；  

app=FaceHuaMaWT.exe  
app表示要更新的程序，若要更新其他程序，只需要将app配置成程序名称；  

update_interval=3600  
update_interval表示更新周期，配置成3600秒，表示自动更新软件1个小时会检查更新，只要将软件最新的project.manifest和软件包内容上传后台，各个酒店的电脑会在1个小时之内进行自动更新，并替换软件并启动。  

重新生成默认配置文件：  
若配置文件给搞坏了，只需将config.ini删除，重启自动更新软件，便会生成一份默认配置文件。  

更新日志：  
v1.3:  
1.启动程序时等待主程序的共享内存释放；  
2.程序只允许运行一个实例。  

v1.2:  
1.修复配置文件config.ini中文不匹配的问题；  
2.修复右下角图标不显示的问题；  
3.修复可能启动失败华码程序的问题；  
4.修复不能拷贝新增文件夹的问题；  
5.修复ESC键退出的问题。  