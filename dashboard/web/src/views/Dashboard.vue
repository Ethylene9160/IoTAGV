<template>
    <div class="dashboard">
        <div class="left-panel">
            <collapsible-card class="primary-card" title="串口管理" :default-open="true">
                <el-row :gutter="20" class="serial-port-controls">
                    <el-col :span="24">
                        <el-select v-model="selectedPort" placeholder="选择串口" class="full-width">
                            <el-option
                                v-for="item in serialPorts"
                                :key="item.port"
                                :label="item.port"
                                :value="item.port"
                            ></el-option>
                        </el-select>
                    </el-col>
                    <el-col :span="8" class="button-col">
                        <el-input v-model="serialBaudRate" type="number" placeholder="波特率"></el-input>
                    </el-col>
                    <el-col :span="8" class="button-col">
                        <el-input v-model="serialDataBits" type="number" placeholder="数据位"></el-input>
                    </el-col>
                    <el-col :span="8" class="button-col">
                        <el-input v-model="serialStopBits" type="number" placeholder="停止位"></el-input>
                    </el-col>
                    <el-col :span="12" class="button-col">
                        <el-button type="primary" @click="scanPorts" class="full-width">扫描</el-button>
                    </el-col>
                    <el-col :span="12" class="button-col">
                        <el-button
                            :type="isConnected ? 'danger' : 'success'"
                            @click="toggleConnection"
                            class="full-width"
                        >
                            {{ isConnected ? '断开' : '连接' }}
                        </el-button>
                    </el-col>
                </el-row>
            </collapsible-card>

            <collapsible-card class="primary-card" title="参数配置" :default-open="true">
                <el-row :gutter="20" class="param-inputs">
                    <el-col :span="12">
                        速度向量长度缩放
                    </el-col>
                    <el-col :span="12">
                        <el-input v-model="arrowLengthFactor" type="number" placeholder="缩放因子"></el-input>
                    </el-col>
                </el-row>
            </collapsible-card>
        </div>
        <div style="width: 2%"></div>
        <div class="middle-panel">
            <collapsible-card class="primary-card" title="坐标面板" :default-open="true">
                <div ref="plotlyContainer" id="plotly-container" class="plotly-container"></div>
            </collapsible-card>
        </div>
        <div style="width: 2%"></div>
        <div class="right-panel">
            <collapsible-card class="primary-card" title="终端管理" :default-open="true">
                <el-select v-model="selected_agent_id" placeholder="选择终端编号" class="full-width">
                    <el-option
                        v-for="agent in agents"
                        :key="agent.id"
                        :label="agent.id"
                        :value="agent.id"
                    ></el-option>
                </el-select>
                <div v-if="selected_agent_id !== null">
                    <p style="text-align: center; font-weight: bold;">终端信息</p>
                    <div class="properties-content">
                        <div class="property-row" v-for="(value, key) in formattedSelectedAgent" :key="key" >
                            <div class="property-name">{{ key }}</div>
                            <div class="property-value">
                                <span v-if="key !== '颜色'">{{ value }}</span>
                                <span v-else :style="{ backgroundColor: value, display: 'inline-block', width: '20px', height: '20px' }"></span>
                            </div>
                        </div>
                    </div>

                    <div class="command-section">
                        <p style="text-align: center; font-weight: bold;">发送指令</p>
                        <el-select v-model="selectedCommand" placeholder="选择指令" class="full-width">
                            <el-option label="设置目标位置" value="1"></el-option>
                            <el-option label="设置速度因子" value="2"></el-option>
<!--                            <el-option label="暂停" value="3"></el-option>-->
<!--                            <el-option label="启动" value="4"></el-option>-->
                        </el-select>
                        <div v-if="selectedCommand === '1'">
                            <el-row :gutter="20" class="command-inputs">
                                <el-col :span="12">
                                    <el-input v-model="targetPositionX" type="number" placeholder="X"></el-input>
                                </el-col>
                                <el-col :span="12">
                                    <el-input v-model="targetPositionY" type="number" placeholder="Y"></el-input>
                                </el-col>
                            </el-row>
                        </div>
                        <div v-if="selectedCommand === '2'">
                            <el-row :gutter="20" class="command-inputs">
                                <el-col :span="24">
                                    <el-input v-model="velocityRatio" type="number" placeholder="速度系数"></el-input>
                                </el-col>
                            </el-row>
                        </div>
                        <el-row :gutter="20" class="button-row">
                            <el-col :span="24">
                                <el-button type="primary" @click="sendCommand(this.selectedCommand)" class="full-width">确认</el-button>
                            </el-col>
                        </el-row>
                        <p style="text-align: center; font-weight: bold;">快捷指令</p>
                        <el-row :gutter="20" class="button-row">
                            <el-col :span="12">
                                <el-button type="primary" @click="sendCommand('3')" class="full-width">暂停</el-button>
                            </el-col>
                            <el-col :span="12">
                                <el-button type="primary" @click="sendCommand('4')" class="full-width">启动</el-button>
                            </el-col>
                        </el-row>
                        <el-row :gutter="20" class="button-row">
                            <el-col :span="12">
                                <el-button type="primary" @click="sendCommand('5')" class="full-width">←</el-button>
                            </el-col>
                            <el-col :span="12">
                                <el-button type="primary" @click="sendCommand('6')" class="full-width">→</el-button>
                            </el-col>
                        </el-row>
                    </div>
                </div>
            </collapsible-card>
        </div>
    </div>
</template>

<script>
import axios from 'axios';
import Plotly from 'plotly.js-dist';
import CollapsibleCard from "@/components/CollapsibleCard.vue";
import { ElMessage } from 'element-plus';
import global_config from "@/config.js";

export default {
    components: {
        CollapsibleCard
    },
    data() {
        return {
            serialPorts: [],
            selectedPort: '',
            serialBaudRate: 115200,
            serialDataBits: 8,
            serialStopBits: 1,
            isConnected: false,
            selected_agent_id: null,
            selected_agent: {},
            agents: [],
            arrowLengthFactor: 0.01,
            selectedCommand: '1',
            targetPositionX: null,
            targetPositionY: null,
            velocityRatio: null
        };
    },
    computed: {
        formattedSelectedAgent() {
            const name_mapping = {
                'id': '编号',
                'position': '位置',
                'velocity': '速度',
                'target_position': '目标位置'
            }
            const formattedAgent = {};
            for (const [key, value] of Object.entries(this.selected_agent)) {
                if (Object.keys(name_mapping).includes(key)) {
                    if (key === 'position' || key === 'velocity' || key === 'target_position') {
                        if (Array.isArray(value) && value.length === 2) {
                            formattedAgent[name_mapping[key]] = value[0].toFixed(2) + ', ' + value[1].toFixed(2);
                        }
                    } else {
                        formattedAgent[name_mapping[key]] = value;
                    }
                }
            }
            return formattedAgent;
        }
    },
    async mounted() {
        this.scanPorts();
        await this.getInitialSerialStatus();

        window.addEventListener('resize', this.resizePlot);

        this.$options.sockets.onmessage = (msg) => {
            const data = JSON.parse(msg.data);

            /*
                画布更新消息: data = {
                    type: 'plot',
                    agents: [
                        {
                            id: id,
                            position: [x, y],
                            (target_position: [tx, ty],)
                            (velocity: [vx, vy],)
                            color: [r, g, b]
                        },
                        ...
                    ],
                    anchors: [
                        {
                            id: id,
                            position: [x, y]
                        },
                        ...
                    ]
                }
                串口状态更新消息: data = {
                    type: 'serial_status',
                    new_status: true / false
                }
            */
            if (data.type === 'plot') {
                this.updatePlot(data.agents.concat(data.anchors));
                this.agents = data.agents;
                this.updateSelectedAgent();
            } else if (data.type === 'serial_status') {
                this.isConnected = data.new_status;
                ElMessage({
                    message: `串口状态更新: ${data.new_status ? '已连接' : '已断开'}`,
                    type: 'info'
                });
            }
        };

        this.initializePlot();
    },
    beforeDestroy() {
        window.removeEventListener('resize', this.resizePlot);
    },
    methods: {
        async scanPorts() {
            try {
                const response = await axios.get((new URL("/serials", global_config.http_address)).href);
                this.serialPorts = response.data.map(serial => ({
                    port: serial.port,
                    desc: serial.desc
                }));
                ElMessage({
                    message: '串口扫描成功',
                    type: 'success'
                });
            } catch (error) {
                console.error('Error scanning ports:', error);
                ElMessage({
                    message: '串口扫描失败',
                    type: 'error'
                });
            }
        },
        async getInitialSerialStatus() {
            try {
                const response = await axios.get((new URL("/serial", global_config.http_address)).href);
                this.isConnected = response.data.status;
            } catch (error) {
                console.error('Error getting initial serial status:', error);
            }
        },
        async toggleConnection() {
            if (!this.selectedPort && !this.isConnected) {
                ElMessage({
                    message: '请选择一个串口',
                    type: 'warning'
                });
                return;
            }

            const serialArgs = {
                opt: this.isConnected ? 'close' : 'open',
                port: this.selectedPort,
                baudrate: this.serialBaudRate,
                bytesize: this.serialDataBits,
                parity: 'N',
                stopbits: this.serialStopBits
            };

            try {
                const response = await axios.post((new URL("/serial", global_config.http_address)).href, serialArgs);
                if (response.data.status) {
                    this.isConnected = !this.isConnected;
                    ElMessage({
                        message: `串口${this.isConnected ? '连接' : '断开'}成功`,
                        type: 'success'
                    });
                } else {
                    ElMessage({
                        message: `串口${this.isConnected ? '断开' : '连接'}失败`,
                        type: 'error'
                    });
                }
            } catch (error) {
                console.error(`Error ${this.isConnected ? 'disconnecting from' : 'connecting to'} port:`, error);
                ElMessage({
                    message: `串口${this.isConnected ? '断开' : '连接'}失败`,
                    type: 'error'
                });
            }
        },
        initializePlot() {
            const plotContainer = this.$refs.plotlyContainer;

            const layout = {
                xaxis: {
                    title: 'x (m)',
                    range: [-1.75, 3.75],
                    scaleanchor: 'y',
                    scaleratio: 1,
                },
                yaxis: {
                    title: 'y (m)',
                    range: [-0.5, 5.5],
                    // scaleanchor: 'x',
                    // scaleratio: 1,
                },
                dragmode: 'pan'
            };

            Plotly.newPlot(plotContainer, [], layout, {
                scrollZoom: true
            });
        },
        updatePlot(points) {
            const plotContainer = this.$refs.plotlyContainer;
            if (plotContainer) {
                // 保存当前的 layout 状态
                const layout = plotContainer.layout || {};

                // 绘制坐标点和目标点的数据
                var data = [];
                for (let item of points) {
                    if ('id' in item && 'position' in item && item.position) {
                        data.push({
                            x: [item.position[0]],
                            y: [item.position[1]],
                            mode: 'markers+text',
                            type: 'scatter',
                            name: `${item.id}`,
                            marker: {
                                size: 12,
                                color: ('color' in item ? `rgb(${item.color[0]}, ${item.color[1]}, ${item.color[2]})` : 'grey')
                            },
                            text: `${item.id}`,
                            textposition: 'top right',
                            hoverinfo: 'none',
                            // hovermode: 'closest',
                            showlegend: false
                        });
                        if ('target_position' in item && item.target_position) {
                            const [tx, ty] = item.target_position;
                            const targetColor = ('color' in item ? `rgba(${item.color[0]}, ${item.color[1]}, ${item.color[2]}, 0.3)` : 'rgba(128, 128, 128, 0.3)');
                            const borderColor = ('color' in item ? `rgba(${item.color[0]}, ${item.color[1]}, ${item.color[2]}, 0.6)` : 'rgba(128, 128, 128, 0.6)');

                            const theta = Array.from({ length: 36 }, (_, i) => i * (2 * Math.PI / 36));
                            const xCircle = theta.map(angle => tx + 0.3 * Math.cos(angle));
                            const yCircle = theta.map(angle => ty + 0.3 * Math.sin(angle));

                            data.push({
                                x: xCircle,
                                y: yCircle,
                                mode: 'lines',
                                type: 'scatter',
                                line: {
                                    width: 2,
                                    color: borderColor,
                                    dash: 'dash'
                                },
                                fill: 'toself',
                                fillcolor: targetColor,
                                hoverinfo: 'none',
                                showlegend: false
                            });
                        }
                    }
                }

                // 绘制箭头的注释
                const annotations = points.map(item => {
                    if ('id' in item && 'position' in item && 'velocity' in item && item.velocity) {
                        const [x, y] = item.position;
                        const [vx, vy] = item.velocity;
                        const endX = x + vx * this.arrowLengthFactor;
                        const endY = y + vy * this.arrowLengthFactor;
                        const arrowColor = ('color' in item ? `rgba(${item.color[0]}, ${item.color[1]}, ${item.color[2]}, 0.6)` : 'rgba(128, 128, 128, 0.6)');

                        return {
                            x: endX,
                            y: endY,
                            ax: x,
                            ay: y,
                            xref: 'x',
                            yref: 'y',
                            axref: 'x',
                            ayref: 'y',
                            showarrow: true,
                            arrowhead: 2,
                            arrowsize: 1,
                            arrowwidth: 2,
                            arrowcolor: arrowColor
                        };
                    } else {
                        return null;
                    }
                }).filter(d => d !== null);

                // 更新图形
                Plotly.react(plotContainer, data, layout);

                // 添加箭头注释
                Plotly.relayout(plotContainer, { annotations: annotations });
            }
        },
        resizePlot() {
            Plotly.Plots.resize(this.$refs.plotlyContainer);
        },
        updateSelectedAgent() {
            const selected = this.agents.find(agent => agent.id === this.selected_agent_id);
            this.selected_agent = selected ? selected : {};
        },
        async sendCommand(sel_cmd) {
            if (!this.selected_agent_id) {
                ElMessage({
                    message: '请先选择一个终端编号',
                    type: 'warning'
                });
                return;
            }

            var commandArgs = {
                type: Number(sel_cmd),
                id: Number(this.selected_agent_id),
                opt1: 0,
                opt2: 0
            };
            if (sel_cmd === '1') {
                commandArgs.opt1 = Number(this.targetPositionX);
                commandArgs.opt2 = Number(this.targetPositionY);
            } else if (sel_cmd === '2') {
                commandArgs.opt1 = Number(this.velocityRatio);
            }

            try {
                const response = await axios.post((new URL("/command", global_config.http_address)).href, commandArgs);
                if (response.data.status) {
                    ElMessage({
                        message: '指令发送成功',
                        type: 'success'
                    });
                } else {
                    ElMessage({
                        message: '指令发送失败',
                        type: 'error'
                    });
                }
            } catch (error) {
                console.error('Error sending command:', error);
                ElMessage({
                    message: '指令发送失败',
                    type: 'error'
                });
            }
        }
    }
};
</script>

<style>
.dashboard {
    display: flex;
    justify-content: center;
    margin-top: 2rem;
    padding: 0 1rem;
}
.left-panel {
    width: 25%;
}
.middle-panel {
    width: 40%;
}
.right-panel {
    width: 25%;
    display: flex;
    flex-direction: column;
    gap: 1rem;
}
.primary-card {
    width: 100%;
}
.serial-port-controls {
    margin-top: 1rem;
}
.command-inputs {
    margin-top: 1rem;
}
.button-col {
    margin-top: 1rem; /* Add margin to create space between rows */
}
.full-width {
    width: 100%;
}
.plotly-container {
    width: 100%;
    height: calc(100vh - 300px); /* Adjust height to fill the viewport height */
}
.properties-content {
    display: flex;
    flex-direction: column;
    align-items: flex-start;
    margin-top: 1rem;
}
.property-row {
    display: flex;
    justify-content: space-between;
    width: 100%;
    margin-bottom: 0.5rem;
}
.property-name {
    flex: 1;
    color: #333;
    /*font-weight: bold;*/
}
.property-value {
    flex: 1;
    color: #666;
    text-align: right;
}
.command-section {
    margin-top: 1rem;
}
.button-col {
    display: flex;
    align-items: center;
}
.button-row {
    margin-top: 1rem; /* 添加间隔 */
}
</style>
