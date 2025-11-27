// 改变图片按钮事件
document.getElementById('改变图片').addEventListener('click', function() {
  const image = document.getElementById('image');
  console.log('Button clicked');  // 打印按钮点击信息
  
  // 判断图片是否是原图，如果是则替换图片，反之恢复原图
  if (image.src.includes('OIP-C.jpg')) {
    image.src = 'https://img-s.msn.cn/tenant/amp/entityid/AA1R62aD.img?w=556&h=668&m=6&x=244&y=107&s=59&d=59';
  } else {
    image.src = 'OIP-C.jpg';
  }
});

// 发送消息按钮事件
document.getElementById('发送消息').addEventListener('click', function() {
  // 发起GET请求到指定URL
  fetch('http://192.168.10.102:8080/sendmsg', {
    method: 'GET',
  })
  .then(response => {
    if (response.ok) {
      return response.json();  // 如果响应是成功的，返回JSON数据
    }
    throw new Error('网络错误');
  })
  .then(data => {
    console.log('Message sent successfully:', data);  // 打印返回的成功信息
  })
  .catch(error => {
    console.error('Error sending message:', error);  // 捕获错误并打印
  });
});
