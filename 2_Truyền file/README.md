# Truyền File qua Socket

## Cách thức truyền file
Đầu tiên ta cần ôn lại cách đọc và ghi file bằng ngôn ngữ C. File được truyền đi sẽ là file bất kỳ chứ không chỉ là mỗi file text đơn thuần nên chế độ đọc và ghi sẽ đều phải ở *binary mode*.

Một số file sẽ có kích thước nhỏ, có thể mang trong một gói tin nhưng một số file lớn sẽ không thể chứa hết trong một gói tin mà phải chia nhỏ làm nhiều phần và gửi đi qua những gói tin khác nhau. Mỗi một phần nhỏ được gọi là một **chunk**.

Mỗi client socket sẽ có một socket buffer riêng. Buffer có thể hiểu là một mảng tạm để lưu dữ liệu được gửi đến.

Ở phía client, ta sẽ đọc dần dữ liệu file từ socket buffer dưới từng chunk nhỏ. Ví dụ, kích thước đọc mỗi chunk là 5 và mảng tạm để lưu nó là ```buf```. Hiện client đó đang đọc thông điệp "Hello world". Vậy giá trị của ```buf``` qua các lần gọi **recv()** sẽ như sau:
```C
char buf[5];
recv(sock, buffer, sizeof(buffer), 0); // "Hello"
recv(sock, buffer, sizeof(buffer), 0); // " worl"
recv(sock, buffer, sizeof(buffer), 0); // "d"
```

Đối với việc tải file qua một link trên internet (ví dụ: http://www.axmag.com/download/pdfurl-guide.pdf), ta sẽ phải thao tác với **HTTP request**. Về cơ bản HTTP request cũng dựa trên TCP, nhưng đã có format được định sẵn để thuận tiện cho việc trao đổi thông tin liên quan tới website. Cổng mặc định của http là cổng 80 (Ví dụ khi ta nhập một IP vào trình duyệt như 127.0.0.1, trình duyệt sẽ tự động gửi một **GET** request tới 127.0.0.1:80). 

Đối với mỗi request, server sẽ trả về HTTP response tương ứng.

Cấu trúc chung của HTTP request hay response đều có 2 phần chính, đó là header và content. Header là để chứa những thông tin về điều khiển hay trạng thái. Trong khi đó content để chứa phần nội dung. Ở việc tải file bằng http request, file trả về sẽ được chứa ở phần content của response. Vì thế ta cần đọc từ đúng phần để ghi nội dung ra hợp lý.

Thường thì content và header sẽ tách nhau ra một dòng, tức là giữa chúng sẽ có một khoảng ```\r\n\r\n```. Ta sẽ sử dụng đặc điểm này để phân tách thông tin header với content.

Ví dụ một HTTP Response trông sẽ như thế này. Ta có thể thấy rõ phần header và content được tách nhau với một dòng trống.
```
HTTP/1.1 200 OK
Connection: Keep-Alive
Keep-Alive: timeout=5, max=100
content-type: text/html
last-modified: Wed, 29 Mar 2023 02:55:02 GMT
accept-ranges: bytes
content-length: 118
date: Tue, 08 Aug 2023 12:40:18 GMT
server: LiteSpeed

<html>
    <head>
        <title>Hello</title>
    </head>
    <body>
        <h1>Hello World</h1>
    </body>
</html>
```

## Truyền file đơn giản giữa 2 process
### Source code: **simple/server/server.c**, **simple/client/client.c**

Mục tiêu phần này là phải thành thạo kỹ thuật truyền và nhận dữ liệu qua socket. Vì thế trước khi bắt đầu phần này, khuyến khích nên thử trước kỹ thuật này với text thông thường (Ví dụ thử viết CT trao đổi text đơn giản giữa 2 bên nhưng bên nhận để chunk nhỏ tầm 3-4 bytes để có thể xem cách hoạt động).

Source code phần này sẽ là về chương trình truyền file đơn giản gồm bên gửi (server) và bên nhận (client). Bên gửi sẽ gửi cho bất cứ client nào mới kết nối đến một file (download.mp3).

Giao thức phần này ta sẽ sử dụng TCP.

Các bước vẫn áp dụng như truyền gói tin cơ bản. Nhưng đối với phần này, thay vì truyền đi text thông thường, ta sẽ truyền dần các **chunk** của file cần truyền. Trong trao đổi file qua mạng, việc truyền hoặc nhận nguyên một file qua gói tin thường ít khi được áp dụng do kích thước buffer của socket có hạn và một số lí do về mục đích sử dụng (*Ví dụ đơn giản khi xem video online, mỗi video có thể nặng tới chục GB liền và ta không muốn đợi trình duyệt load xong cả chục GB rồi mới bắt đầu xem được*).

Ở phía server, ta tiến hành đọc file và xác nhận file có tồn tại trước để tránh lỗi. Sau đó ta tiến hành đọc dữ liệu file vào buffer và truyền đi dần dần.
```C
int len = NULL;
int total = 0;
while ((len = fread(buffer, 1, BUFFER_SIZE, file)) > 0) {
    send(client, buffer, len, 0);
    total = total + len;
}
```

Phía client sẽ nhận được các chunk dữ liệu gửi từ server. Các chunk này sẽ được đẩy vào buffer của client socket. Ta sẽ tiến hành lấy dữ liệu từ buffer của socket qua hàm **recv()** và ghi ra file.
```C
int len;
while ((len = recv(sock, buffer, sizeof(buffer), 0)) > 0) {
    fwrite(buffer, 1, len, f);
    size += len;
}
``` 

## Tải file về qua HTTP request
### Source code: **download.c**

Trước khi đi sâu vào phần này, cần ôn lại về cấu trúc một HTTP request và HTTP response.

Mục tiêu phần này sẽ là tải về một file từ internet qua giao thức HTTP.

Ở phần này, ta sẽ đóng vai trò làm một client để yêu cầu file từ server. Như vậy cũng làm tương tự đối với việc giao tiếp với server bằng TCP, bước đầu ta cũng phải thiết lập kết nối trước. Tuy nhiên, việc thiết lập kết nối sẽ phức tạp hơn khi ta chưa có trước địa chỉ IP của server mà chỉ có duy nhất hostname (VD như *lebavui.id.vn*).

Ta có thể sử dụng hàm sau để làm việc đó. Hàm sẽ tự động lấy những thông tin tương ứng với một hostname để lưu kết quả vào một ```struct addrinfo```.

```C
int getaddrinfo(const char *nodename,
                const char *servname,
                const struct addrinfo *hints,
                struct addrinfo **res);
```
*[Cách dùng](https://www.ibm.com/docs/en/zos/2.4.0?topic=functions-getaddrinfo-get-address-information)*

Sau khi thiết lập xong kết nối, ta sẽ quan tâm đến việc gửi đi request ra sao (cụ thể ở đây là HTTP request) để nhận được kết quả mong muốn.

Dữ liệu được gửi đi phải dưới dạng một http request. Trong ví dụ này, ta muốn tải file ở địa chỉ sau đây về máy www.axmag.com/download/pdfurl-guide.pdf, vì vậy request đơn giản sẽ có dạng như sau:
```
GET /download/pdfurl-guide.pdf HTTP/1.1
Host: www.axmag.com
Connection: close
```
Vì thế thông điệp được gửi đi dưới dạng xâu sẽ là ```"GET /download/pdfurl-guide.pdf HTTP/1.1\r\nHost: www.axmag.com\r\nConnection: close\r\n\r\n"```

Sau khi server nhận được yêu cầu thì sẽ tiến hành trả về kết quả hay response. HTTP response gồm có header và content, ta cần tách chúng ra và chỉ ghi phần content vào file mới. 

```
HTTP Response:
    - Header (Thông tin của response)

    - Content (Phần này chính là dữ liệu file ta cần tải về)
```

Việc tách header ra sẽ sử dụng đến đặc điểm phân cách như đã nói ở trên. Tức là ta sẽ liên tục  đọc dần dữ liệu header cho tới khi dữ liệu chúng ta lưu (ở ví dụ là ```res```) có xuất hiện ```\r\n\r\n``` thì dừng lại (tức là đã có tách dòng) và đánh dấu kết thúc header.

```C
while (1) {
    int len = recv(client, buf, sizeof(buf), 0);
    if (len <= 0) {
        printf("Failed to download file.\n");
        close(client);
        exit(1);
    }

    res = (char *)realloc(res, size + len + 1);
    memcpy(res + size, buf, len);
    size += len;
    res[size] = '\0';

    if ((pos = strstr(res, "\r\n\r\n")) != NULL) {
        break;
    }
}
```

Phần còn lại chính là lấy nốt dữ liệu để lưu vào file qua đọc ghi file. Lưu ý là với cách lấy header như trên, có thể có một phần của content đã bị đọc vào ```res``` mà trong quá trình về sau ghi ra file ta không thể lấy lại được qua **recv()**. Vì thế để có file hoàn chỉnh ta cần ghi lại phần đó vào file trước rồi mới đọc tiếp từ buffer.

```C
/*
Do trong bộ nhớ, sau /r/n/r/n vẫn còn một phần nhỏ dữ liệu content đã bị kèm cả vào nên
ta cần khôi phục. Hiện pos đang trỏ vào ký tự '\r' đầu tiên, như vậy dịch thêm 4 ký tự
sẽ trỏ tới byte đầu tiên của content. Kích thước lý thuyết của header đã nhận là size.
Vì thế phần content thừa sẽ là từ pos + 4 cho tới res + size. Ta cần ghi lại phần này 
vào file.
*/
if (size - (pos - res) - 3 > 0) fwrite(pos + 4, 1, size - (pos - res) - 3, f);
```

Và cuối cùng là ghi phần còn lại vào và kết thúc ghi file

```C
while (1) {
    int len = recv(client, buf, sizeof(buf), 0);
    if (len <= 0) {
        break;
    }

    fwrite(buf, 1, len, f);
    size += len;
}

fclose(f);
```

## Kiểm tra độ tin cậy
Đôi khi yêu cầu của việc truyền file cần độ chính xác tuyệt đối, tức là không thể có lỗi trong quá trình truyền và nhận. Vì vậy luôn luôn có nhu cầu cần kiểm tra xem file gốc và file nhận có giống nhau hay không.

Công cụ phổ biến để thực hiện việc này đó là dựa vào MD5 checksum. Nếu chuỗi checksum của file nhận được và file gửi đi giống nhau tức là file đã được truyền đi nguyên vẹn. Ngược lại cho dù ta cảm thấy file nhận được đã hoàn thiện (ví dụ file audio đã nghe giống với file gốc, ảnh trông giống ảnh gốc) nhưng nếu chuỗi checksum khác nhau tức là file chưa được truyền đi toàn vẹn hay là đã có lỗi xảy ra.

Ở trên Linux ta có thể trực tiếp sử dụng **md5sum** để kiểm tra vấn đề này.
```
[huy@localhost simple_file]$ md5sum client/download.mp3
aee76af495ea5ff82bf8b10aa9084159  client/download.mp3
[huy@localhost simple_file]$ md5sum server/download.mp3
aee76af495ea5ff82bf8b10aa9084159  server/download.mp3
```

Hai chuỗi checksum cùng có giá trị ```aee76af495ea5ff82bf8b10aa9084159```, ta kết luận 2 file là giống nhau.

