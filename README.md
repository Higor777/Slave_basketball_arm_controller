# Slave_basketball_arm_controller
This is the second version of the basketball arm controller(SLAVE).
<h2>第二代篮球机器人底盘STM32F1里程计程序</h2>
<p><font size="10px">底盘为三轮全向结构</font></p>
<p>
    <ul>
        <li>TIM3、TIM4、TIM5采用正交解码计数三轮编码器速度脉冲</li>
        <li>对各轮速度脉冲数积分得到各轮脉冲里程数</li>
        <li>与STM32F4采用CAN通信，实现里程清零与里程请求</li>
    </ul>
</p>
