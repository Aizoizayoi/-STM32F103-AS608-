版本说明：
6.2  单独把as608_workshop提取了出来，main仅调动函数
6.3  接收模式从receive_IT改成DMA，应答判断从RxEventCallback里进行改成：RxEventCallback接收完完整的应答后，在单独的函数里判断。降低中断的时间，并适配DMA，方便移植。
	 中断回调函数中接一个字节判断一个字节修改成：中断回调函数仅接收和只复制转移接收的数据，合法性判断由单独的的函数，在发送指令后进行

修改详情：
1.AS608_SendData修改成HAL_UART_Transmit发送
2.每条指令前增加AS608_BeginReceive();初始化和开启接收不定长应答包
3.VertifyAck简化，仅对应答接收进行超时判断、是否接收完成的判断，合法性通过调用AS608_AckVertify()函数进行，
  删除了VertifyAck内部的头验证，在无异常返还数据指针。
4.AS608_AckVertify对完整接收后的数据进行合法性判断，非状态机，依次判断2字节包头、4字节地址、1字节包标识
和2字节包长，对包长进行验证后，若合法则标志并返回AS608_Data_AckOK的验证状态；非法则返回各字节出错时的对应标识
5.新增usart_callback函数，使各个函数可以用于独立的文档，来编写回调函数的程序
6.USART2_RxEventCallback仅接收数据，并进行长度的合法性验证和转存。
7.新增as608_workshop.c，封装对as608的操作菜单