# Chuẩn bị
## A. Cài đặt môi trường
* Toàn bộ source code và hướng dẫn sẽ đều dựa trên lập trình trên **Linux**. (*Lưu ý: Nhiều chức năng sẽ được thực hiện theo cách khác nhau với thư viện và hàm khác nhau giữa Window và Linux, vì thế source code có thể chạy trên Linux nhưng sẽ có thể không hoạt động nếu chạy ở Window*) Vì thế cần cài trước hoặc sử dụng máy ảo.
* Ôn lại về compiler **GCC** (**G++** để biên dịch mã **C++**) và cách biên dịch mã của một file code (*.c* hoặc *.cpp*) sang **binary file** và chạy file đó.
* VD:
```
gcc ma_nguon.c -o ten_file_dich
./ten_file_dich
```

## B. Kiến thức về socket
* Ôn tập lại khái niệm **Socket** là gì? Nó nằm ở đâu trong mô hình OSI.
* Ôn tập lại về giao thức **TCP/IP** và **UDP**, cơ chế hoạt động và trao đổi gói tin của các giao thức đó.

## B. Lập trình C cơ bản
* Để hiểu và nắm được lập trình socket cơ bản và viết chương trình liên quan, ta cần nắm được kiến thức cơ bản về C, nhất là kiến thức liên quan tới mảng, tới việc ghi, đọc file,...