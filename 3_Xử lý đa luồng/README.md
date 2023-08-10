# Xử lý đa luồng
## Đặt vấn đề
Quay trở lại chương trình TCP ở phần 1 (truyền gói tin cơ bản). Đầu tiên ta chạy chương trình server và client như bình thường. Sau đó ở tab thứ 3 hãy thử chạy một chương trình client thứ 2, thử giao tiếp với server và quan sát kết quả.

Client đầu tiên sẽ giao tiếp với server một cách bình thường. Nhưng ở client thứ 2, ta sẽ thấy mặc dù ta gửi đi nhiều gói tin nhưng không có phản hồi từ phía server.

Bây giờ, ở client đầu tiên, gõ "QUIT" để thoát ra, sau đó quay trở lại client thứ 2. Ta mới thấy server đáp lại toàn bộ các gói tin mà client 2 đã gửi theo thứ tự.

Sở dĩ hoạt động như vậy là do server chấp nhận kết nối mới thông qua hàm **accept()**. Hàm sẽ trả về socket descriptor để server có thể giao tiếp với client ở đầu hàng đợi. Trong khi chạy client 1, phía server chỉ đang xử lý công việc của hàm ```void* newClient(int client)```. Sau khi kết thúc phiên với client 1, vòng lặp while chính mới được lặp lại. 

```C
while (1) {	
    client =  accept(sock, NULL, NULL);
    newClient(client);
}	
```

Lần này client 2 đang đứng đầu hàng đợi nên hàm **accept()** sẽ giúp server và client 2 có thể giao tiếp với nhau. Những gói tin mà client 2 đã gửi trước khi được accept sẽ nằm trong buffer của client 2. Sau khi thiết lập thành công sẽ tự động gửi đi.

Nhưng trong các ứng dụng thực tế, một server sẽ phải phục vụ nhiều client cùng một lúc. Vì thế, ta cần tiếp cận theo hướng đa luồng.

Mục tiêu trong phần này sẽ là cải tiến chương trình server trên để nhiều client có thể kết nối đến cùng một lúc và giao tiếp bình thường, không phải đợi client khác kết thúc phiên rồi mới đến lượt mình như phía trên.

Hay nói cách khác, ta phải đảm bảo hàm **accept()** luôn sẵn sàng đối với số lượng client nhất định để mỗi khi có client mới kết nối đến sẽ ngay lập tức được chấp nhận. 

Ở phiên bản ban đầu, sở dĩ **accept()** không thể chấp nhận tới client thứ 2 là do hàm chỉ chạy được sau khi hàm **newClient(client)** kết thúc. Do đó, để hàm **accept()** không phải đợi nữa, mỗi một lần hàm  **newClient(client)** chạy sẽ phải ở một luồng riêng biệt, tách hẳn ra so với luồng chính. 

## Đa luồng trong C
Đa luồng là kỹ thuật chạy nhiều tác vụ cùng một lúc trong cùng một chương trình. Điều này cho phép việc sử dụng tài nguyên xử lý của máy tính một cách hiệu quả hơn cũng như hoàn thành được nhiều công việc cần yêu cầu đa luồng.

Đa luồng (multi threading) không được hỗ trợ bởi bản thân ngôn ngữ C mà phải dựa vào tính năng mà hệ điều hành cung cấp. Đối với Linux, ta sẽ sử dụng POSIX Thread hay Pthread. Và các hàm cơ bản để xử lý về thread của Pthread đều có trong thư viện ```pthread.h```.

Để hình dung cụ thể hơn về luồng, cùng xét ví dụ dưới đây
```C
#include <stdio.h>
#include <unistd.h>


void print_foo() {
    int i;
    for (i = 0; i < 3; i++) {
        printf("Foo print: %d\n", i);
        sleep(0.1);
    }
    
}

void print_bar() {
    int i;
    for (i = 3; i < 6; i++) {
        printf("Bar print: %d\n", i);
        sleep(0.1);
    }
}

int main() {
    print_foo();
    print_bar();
    return 0;
}
```

Ở đoạn chương trình trên, lần lượt hàm ```print_foo()``` và ```print_bar()``` sẽ được thực hiện, kết quả trả về sẽ như sau

```
Foo print: 0
Foo print: 1
Foo print: 2
Bar print: 3
Bar print: 4
Bar print: 5
```

Bây giờ, nếu muốn cả hai hàm trên cùng chạy song song, ta sẽ phải tạo thread tương ứng với từng hàm để chạy chúng.

```C
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

void* print_foo(void* arg) {
    int i;
    for (i = 0; i < 3; i++) {
	printf("Foo print: %d\n", i); 
	sleep(1);
    }
}

void* print_bar(void* arg) {
    int i;
    for (i = 3; i < 6; i++) {
	printf("Bar print: %d\n", i); 
	sleep(1);
    }
}


int main() {
    pthread_t tid[2];
    pthread_create(&tid[0], NULL, print_bar, NULL);
    pthread_create(&tid[1], NULL, print_foo, NULL);

    pthread_join(tid[1], NULL);
    return 0;
}
```

* Trong đó:
    * **pthread_t** là kiểu dữ liệu để lưu giá trị đại diện cho một luồng (thread) hay nói cách khác là giá trị định danh. Về sau các thao tác liên quan đến một luồng cụ thể nào đó sẽ đều phải dựa vào giá trị đại diện này của nó.

    ```C
    int pthread_create(pthread_t *thread, pthread_attr_t *attr,
                    void *(*start_routine) (void *arg), void *arg);    
    ```

    * **pthread_create()** là hàm để tạo một thread. Tham khảo cách dùng cụ thể [ở đây](https://www.ibm.com/docs/en/zos/2.3.0?topic=functions-pthread-create-create-thread). Nhưng tóm tắt chính thì hàm yêu cầu 4 tham số:
        * ***thread*** là con trỏ tới định danh luồng (*pthread_t \**). 
        * ***attr*** liên quan tới các thuộc tính (attributes), nếu không cần ta có thể để là NULL.
        * ***start_routine*** là hàm sẽ chạy của luồng đó. Lưu ý hàm phải có kiểu trả về là ```void *``` và tham số của hàm chỉ có một và duy nhất một tham số kiểu ```void *```.
        * ***arg*** là tham số của hàm.

    * ***pthread_join()***: ```pthread_join(tid[1], NULL);```  như trong ví dụ trên sẽ dừng luồng chính của chương trình cho tới khi luồng có định danh là *tid[1]* kết thúc thì luồng chính mới chạy tiếp. Ở trong ví dụ trên, nếu bỏ dòng này đi, kết quả của chương trình sẽ không có gì cả. Vì nếu luồng chính kết thúc thì dù những luồng con chạy chưa xong nhưng cũng sẽ bị hủy. Vì thế, ta muốn đợi luồng chạy hàm ```print_foo()``` kết thúc thì mới kết thúc chương trình.

Kết quả của đoạn chương trình trên
```
Bar print: 3
Foo print: 0
Bar print: 4
Foo print: 1
Bar print: 5
Foo print: 2
```

## Áp dụng vào ví dụ truyền gói tin cơ bản
### Source code: multithread_tcp/server.c
Như đã đề cập ở phần đầu, ta muốn tách ra thành luồng con cho hàm **newClient(client)**. Kỹ thuật về luồng thì áp dụng như ví dụ phần cơ bản về luồng phía trên. Tuy nhiên, hàm **newClient(client)** yêu cầu tham số. Vì thế khác với ví dụ trước, ta phải truyền 1 tham số là giá trị **socket descriptor** vào hàm trên khi tạo luồng.

Do phần này chỉ tập trung xử lý về luồng nên những bước cơ bản về thiết lập kết nối và giao tiếp sẽ được bỏ qua.

Trước khi bắt đầu "nâng cấp" server, ta nên định trước số client tối đa có thể kết nối tới server để phục vụ cho việc xác định kích thước mảng lưu trữ.

```C
#define MAX_CLIENT 10
```

Do mỗi client kết nối đến sẽ đại diện cho một luồng con riêng biệt, socket descriptor để giao tiếp với riêng biệt nên ta cần lưu trữ tất cả các giá trị đó và vị trí lưu trữ hiện tại của mảng. 

```C
pthread_t client_thread[MAX_CLIENT];
int server_sock[MAX_CLIENT];
int client_num = 0;
```

Ví dụ client đầu tiên sẽ có định danh luồng là **client_thread[0]**, socket descriptor để giao tiếp là **server_sock[0]** và để client tiếp theo kết nối thì dữ liệu sẽ được lưu ở vị trí kế tiếp, ta sẽ tăng biến đếm **client_num** lên 1 đơn vị.

```C
server_sock[client_num] =  accept(sock, NULL, NULL);
pthread_create(client_thread + client_num, NULL, newClient, (void*) &server_sock[client_num]);
client_num++;
```

Lưu ý theo hàm để chạy trong luồng giá trị trả về chỉ có thể là *void \** và giá trị tham số cũng tương tự. Vì vậy, trong trường hợp ví dụ này, ta không thể truyền trực tiếp tham số socket descriptor vào được mà phải sử dụng kỹ thuật **casting**.

Cụ thể, mục tiêu của ta là truyền được vào tham số socket descriptor có kiểu là int vào trong hàm chạy của luồng khi bắt đầu tạo. Trong trường hợp này, giá trị đó là **server_sock[client_num]**. Hàm **new_client()** bắt buộc phải có giá trị này thì mới có thể giao tiếp được với client. Nếu áp dụng casting, ta có thể cast trực tiếp để chuyển từ kiểu *int* thành *(void \*)* và sau đó ở trong hàm **new_client()** sẽ chuyển ngược lại để ra giá trị **server_sock[client_num]**.

Tuy nhiên, việc làm như vậy không được khuyến khích do kích thước của kiểu *int* và *(void \*)* khác nhau. Thay vì vậy, ta sẽ truyền vào địa chỉ của **server_sock[client_num]**, tức là **&server_sock[client_num]** với kiểu dữ liệu là *(int \*)*. Và sau đó chuyển ngược lại trong hàm **newClient** để lấy được giá trị socket descriptor đã được truyền vào.

```C
void* newClient(void* client_param) {
	int client = * (int*) client_param;
    //...............
}
```

Ngoài ra thì chương trình client có thể giữ nguyên

## Xử lý xung đột khi truy xuất dữ liệu
Nhắc đến vấn đề đa luồng thì không thể không nhắc tới vấn đề xung đột về tài nguyên. Khi nhiều luồng khác biệt cùng truy cập và sửa đổi vào một tài nguyên trên server.

Ví dụ: *Bạn còn 300$ trong tài khoản ngân hàng. Một ngày nọ, bạn đi rút 50$ để tiêu. Nhưng trùng hợp trong khi bạn rút thì một người bạn cũ trả nợ bạn nên chuyển 100$ vào tài khoản của bạn. Cả 2 yêu cầu cùng lúc được gửi tới server của ngân hàng và hơn nữa cùng truy cập vào cùng một tài nguyên với giá trị ban đầu là 300$. Như vậy, 2 luồng tạo ra bởi 2 yêu cầu sẽ có 2 giá trị mới cho tài khoản là 250$ và 400$. Điều này dẫn tới xung đột.*

Chính vì thế ta cần công cụ để quản lý xung đột. Để minh họa cách tiến hành, hãy cùng xét ví dụ về tài khoản ngân hàng phía trên.

```C
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

int credit = 300;

void* withdraw(void* arg) {
    printf("Before withdrawing, I had %d$\n", credit); 
    int value = credit;
    printf("Withdrawing...\n");
    sleep(1);
    value -= 50;
    printf("Now I have %d$\n", value);
    credit = value; 
}

void* transfer(void* arg) {
    printf("Before transfering, he had %d$\n", credit);
    int value = credit;
    printf("Transfering...\n");
    sleep(1);
    value += 100;
    printf("Now he has %d$\n", value); 
    credit = value; 
}


int main() {
    pthread_t tid[2];
    pthread_create(&tid[0], NULL, transfer, NULL);
    pthread_create(&tid[1], NULL, withdraw, NULL);

    pthread_join(tid[1], NULL);
    printf("---------------------------\nFinally, your credit is %d$\n", credit);
    return 0;
}
```

Và kết quả như dưới đây.
```
Before transfering, he had 300$
Transfering...
Before withdrawing, I had 300$
Withdrawing...
Now he has 400$
Now I have 250$
---------------------------
Finally, your credit is 250$

```

Cuối cùng, số dư của bạn là 250$. Và 100$ kia đã rơi vào tay ngân hàng chứ không thuộc về bạn!

Như vậy để giúp cho việc truy cập đồng thời không bị mâu thuẫn, ta sẽ tiến hành mỗi khi một luồng nào đó truy cập số dư của bạn để xử lý, ta sẽ tiến hành khóa tài nguyên đó lại để không cho luồng khác truy cập cho tới khi thực hiện xong việc xử lý thì luồng tiếp theo mới được phép tiến hành trên tài nguyên ấy. 

Ở trong trường hợp trên, giả sử ngân hàng nhận được yêu cầu chuyền tiền trước một chút. Hệ thống sẽ khóa vùng tài nguyên liên quan đến số dư của bạn cho tới khi cập nhật xong số dư thì mới mở khóa. 

Yêu cầu rút tiền của bạn cũng cần khóa vùng tài nguyên tương tự. Tuy nhiên, khi vùng tài nguyên ấy vẫn đang trong quá trình xử lý của yêu cầu trước, việc khóa của yêu cầu rút tiền sẽ bị trì hoãn cho tới khi yêu cầu chuyển tiền mở khóa vùng tài nguyên đó.

Đối với pthread, khóa chức năng như vậy là **mutex**. Ta có thể có nhiều khóa phụ trách cho nhiều vùng tài nguyên khác nhau. Ta tiến hành áp dụng khóa mutex vào ví dụ trên.

```C
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

int credit = 300;
pthread_mutex_t credit_mutex;

void* withdraw(void* arg) {
    pthread_mutex_lock(&credit_mutex);
    printf("Before withdrawing, I had %d$\n", credit); 
    int value = credit;
    printf("Withdrawing...\n");
    sleep(1);
    value -= 50;
    printf("Now I have %d$\n", value);
    credit = value; 
    pthread_mutex_unlock(&credit_mutex);
}

void* transfer(void* arg) {
    pthread_mutex_lock(&credit_mutex);
    printf("Before transfering, he had %d$\n", credit);
    int value = credit;
    printf("Transfering...\n");
    sleep(1);
    value += 100;
    printf("Now he has %d$\n", value); 
    credit = value;
    pthread_mutex_unlock(&credit_mutex);
}


int main() {
    pthread_mutex_init(&credit_mutex, NULL);

    pthread_t tid[2];
    pthread_create(&tid[0], NULL, transfer, NULL);
    pthread_create(&tid[1], NULL, withdraw, NULL);

    pthread_join(tid[1], NULL);
    printf("---------------------------\nFinally, your credit is %d$\n", credit);
    return 0;
}
```

Bây giờ số dư của bạn đã chính xác.

```
Before transfering, he had 300$
Transfering...
Now he has 400$
Before withdrawing, I had 400$
Withdrawing...
Now I have 350$
---------------------------
Finally, your credit is 350$
```

Trong chương trình trên, biến có kiểu **pthread_mutex_t** sẽ có vai trò định danh cho khóa mutex. Trong ví dụ trên, ta chỉ cần một khóa là đủ với tên biến là *credit_mutex*. Hàm **pthread_mutex_init()** có vai trò khởi tạo cho khóa của ta là *credit_mutex*. 

Hai hàm **pthread_mutex_lock** và **pthread_mutex_unlock** có vai trò khóa và mở khóa. Khi tiến hành khóa, có 2 trường hợp có thể xảy ra tương ứng với hành vi của **pthread_mutex_lock**:
* Khóa mutex đích chưa bị khóa: Tiến hành khóa khóa mutex đó lại
* Khóa mutex đích đã bị khóa: Hay nói cách khác, đang có luồng khác sử dụng tài nguyên đó, như vậy hàm **pthread_mutex_lock** sẽ tiến hành đợi cho tới khi khóa đích được mở thì mới tiến hành khóa rồi luồng đó mới chạy tiếp

Nhờ cơ chế như vậy mà ta có thể tiến hành kiểm soát truy cập tài nguyên.

