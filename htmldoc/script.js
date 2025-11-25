document.getElementById('changeImageBtn').addEventListener('click', function() {
  const image = document.getElementById('image');
  console.log('Button clicked');  // 打印按钮点击信息
  
  if (image.src.includes('OIP-C.jpg')) {
    image.src = 'https://img-s.msn.cn/tenant/amp/entityid/AA1R62aD.img?w=556&h=668&m=6&x=244&y=107&s=59&d=59';
  } else {
    image.src = 'OIP-C.jpg';
  }
});
