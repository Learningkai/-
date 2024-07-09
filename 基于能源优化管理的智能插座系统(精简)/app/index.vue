<template>
	<scroll-view scroll-y="true" style="height: 100%;">
		<view class="wrap">
			<view class="real-time">{{ currentTime }}</view>
			<view class="dev-area">
				<view class="dev-cart">
					<view class="" @tap="onCurrentClick">
						<view class="dev-name">电流</view>
						<image class="dev-logo" src="../../static/cur.png"></image>
					</view>
					<view class="dev-data">{{ Current }} A</view>
				</view>
				<view class="dev-cart">
					<view class="">
						<view class="dev-name">消耗电量</view>
						<image class="dev-logo" src="../../static/vol.png" mode=""></image>
					</view>
					<view class="dev-data">{{ Electricity.toFixed(2) }} kWh</view>
				</view>
				<view class="dev-cart">
					<view class="">
						<view class="dev-name">功率</view>
						<image class="dev-logo" src="../../static/pow.png" mode=""></image>
					</view>
					<view class="dev-data">{{ Power.toFixed(2) }} W</view>
				</view>
				<view class="dev-cart">
					<view class="">
						<view class="dev-name">总电费</view>
						<image class="dev-logo" src="../../static/exp.png" mode=""></image>
					</view>
					<view class="dev-data">{{ electricityBill.toFixed(2) }} 元</view>
				</view>
				<view class="dev-cart">
					<view class="">
						<view class="dev-name">开关</view>
						<image class="dev-logo" src="../../static/swit.png" mode=""></image>
					</view>
					<switch :checked="led" @change="onLedSwitch" color="#3cb371" />
				</view>
				<view class="dev-cart">
					<view class="">
						<view class="dev-name">Usb开关</view>
						<image class="dev-logo" src="../../static/USB.png" mode=""></image>
					</view>
					<switch :checked="usb" @change="onUsbSwitch" color="#3cb371" />
				</view>
				<view class="dev-adi">
					<view class="">
						<view class="dev-name">建议</view>
						<image class="dev-logo" src="../../static/jianyi.png" mode=""></image>
					</view>
					<view class="dev-data">{{ advice }}</view>
				</view>
			</view>
		</view>
	</scroll-view>
</template>

<script>
	const {
		createCommonToken
	} = require('@/key.js')

	export default {
		data() {   //定义数据
			return {
				Electricity: 0,
				currentTime: '',
				Current: 2,
				Voltage: 220,
				Power: 0,
				led: true,
				usb: true,
				advice: '',
				electricityBill: 0,
				powerData: [],
				token: '',
				electricityt: '',
			}
		},
		mounted() {
			this.updateTime();
			this.generateToken();
			this.fetchDevData();
			this.fetchDevData1();
			setInterval(() => {
				this.ReEledata(this.Electricity);
			}, 60000);   //每分钟反馈一次耗电量数据
			setInterval(() => {
				this.fetchDevData();
				this.updateTime();
			},1000);   //每秒获取一次数据
		},
		methods: {
			generateToken() {   //生成Token令牌
				const params = {   //生成令牌所需信息
					author_key: '8l7gOpK9+aNbu/9wzUj82/w61lgPoGZEMZSMBeTgFhjb/fYFVct89Bgc/5/XqPSZWf2giaGJhiyXnFYOR1YUeQ==',
					version: '2022-05-01',
					user_id: '392748',
				}
				this.token = createCommonToken(params);
			},
			fetchDevData1() {   //获取耗电量函数
				uni.request({
					url: 'https://iot-api.heclouds.com/thingmodel/query-device-property',
					method: 'GET',
					data: {
						product_id: 'YN6kc7Q2C1',
						device_name: 'c1',
					},
					header: {
						'authorization': this.token
					},
					success: (res) => {
						this.Electricity += parseFloat(res.data.data[1].value);
					}
				});
			},
			fetchDevData() {   //获取其他数据函数
				if (!this.token) {   //判断令牌是否生成
					console.error('Token is not available.');
					return;
				}
				uni.request({
					url: 'https://iot-api.heclouds.com/thingmodel/query-device-property',
					method: 'GET',
					data: {
						product_id: 'YN6kc7Q2C1',
						device_name: 'c1',
					},
					header: {
						'authorization': this.token,
					},
					success: (res) => {
						if (res.data && res.data.data) {
							console.log(res.data);
							this.Current = Math.max(res.data.data[0].value, 0); // 电流小于0计为0
							this.Power = this.Current * this.Voltage;
							this.led = res.data.data[2].value === 'true';
							this.usb = res.data.data[3].value === 'true';
							this.Electricity += this.Power / 1000 /3600;   // 计算消耗电量，假设时间间隔为1s
							if (this.Current > 5) {   //判断用电情况设置建议
								this.advice = "当前功率过大，请注意节约用电。";
							} else {
								this.advice = "当前功率正常，请继续保持。";
							}
							this.calculateElectricityBill();   // 计算电费
							this.powerData.push(this.Power);   // 将功率数据添加到数组中，并保持最新的60个数据点
							if (this.powerData.length > 60) {
								this.powerData.shift();
							}
						} else {
							console.error('Invalid response data:', res.data);
						}
					},
					fail: (err) => {
						console.error('Request failed:', err);
					}
				});
			},
			calculateElectricityBill() {   //计算电费函数
				const now = new Date();   //获取当前时间，用于判断高峰低峰
				const hours = now.getHours();
				const rate = (hours >= 8 && hours < 22) ? 0.568 : 0.288;   //高峰和低谷电价
				this.electricityBill = this.Electricity * rate;   //计算
			},
			updateTime() {   //实时更新时间
				const now = new Date();
				this.currentTime = now.toLocaleString();
			},
			onLedSwitch(event) {   //更新电源开关状态函数
				let value1 = event.detail.value;
				uni.request({
					url: 'https://iot-api.heclouds.com/thingmodel/set-device-property',
					method: 'POST',
					data: {
						product_id: 'YN6kc7Q2C1',
						device_name: 'c1',
						params: {
							"led": value1,
						}
					},
					header: {
						'authorization': this.token
					},
					success: (res) => {
						console.log('led ' + (value1 ? 'ON' : 'OFF') + '!');
					}
				});
			},
			onUsbSwitch(event) {   //更新usb开关状态函数
				let value2 = event.detail.value;
				uni.request({
					url: 'https://iot-api.heclouds.com/thingmodel/set-device-property',
					method: 'POST',
					data: {
						product_id: 'YN6kc7Q2C1',
						device_name: 'c1',
						params: {
							"usb": value2,
						}
					},
					header: {
						'authorization': this.token
					},
					success: (res) => {
						console.log('usb ' + (value1 ? 'ON' : 'OFF') + '!');
					}
				});
			},
			ReEledata(electricity) {   //回馈耗电量
				let doubleElectricity = parseFloat(electricity);
				let stepElectricity = Math.round(doubleElectricity * 100) / 100;   //令耗电量计算到小数点后两位
				uni.request({
					url: 'https://iot-api.heclouds.com/thingmodel/set-device-property',
					method: 'POST',
					data: {
						product_id: 'YN6kc7Q2C1',
						device_name: 'c1',
						params: {
							"ele": stepElectricity
						}
					},
					header: {
						'authorization': this.token
					},
					success: (res) => {
						console.log('return success!');
						console.log(res);
					}
				});
			},
		}
	}
</script>

<style>
	page,
	body {
		background: url('../../static/cover1.png') no-repeat center center fixed;
		background-size: cover;
	}

	.wrap {
		padding: 30rpx;
	}

	.content {
		display: flex;
		flex-direction: column;
		align-items: center;
		justify-content: center;
	}

	.real-time {
		text-align: center;
		font-size: 24rpx;
		margin-bottom: 20rpx;
		color: #FFFFFF;
	}

	.dev-cart {
		height: 150rpx;
		width: 320rpx;
		border-radius: 30rpx;
		margin-top: 30rpx;
		display: flex;
		justify-content: space-around;
		align-items: center;
		box-shadow: 0 0 15rpx #ccc;
	}

	.dev-adi {
		height: 150rpx;
		width: 750rpx;
		border-radius: 30rpx;
		margin-top: 30rpx;
		display: flex;
		justify-content: space-around;
		align-items: center;
		box-shadow: 0 0 15rpx #ccc;
	}

	.dev-name {
		font-size: 20rpx;
		text-align: center;
		color: #FFFFFF;
	}

	.dev-logo {
		width: 70rpx;
		height: 70rpx;
		margin-top: 10rpx;
	}

	.dev-data {
		font-size: 40rpx;
		color: white;
	}

	.dev-area {
		display: flex;
		justify-content: space-between;
		flex-wrap: wrap;
	}

	.container {
		display: flex;
		flex-direction: column;
		align-items: center;
		justify-content: center;
		height: 100%;
	}
</style>