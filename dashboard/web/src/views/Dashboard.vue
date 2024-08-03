<template>
    <div class="dashboard">
        <div class="left-panel">
            <collapsible-card class="primary-card" title="坐标面板" :default-open="true">
                <div ref="plotlyContainer" id="plotly-container" class="plotly-container"></div>
            </collapsible-card>
        </div>
        <div style="width: 2%"></div>
        <div class="right-panel">
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

            <collapsible-card class="primary-card" title="属性" :default-open="true">
                <div class="properties-content">
                    <!-- <div class="property-row" v-for="(value, key) in selected_agent" :key="key">
                        <div class="property-name">{{ key }}</div>
                        <div class="property-value">{{ value }}</div>
                    </div> -->
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
            isConnected: false,
            selected_agent_id: null
        };
    },
    async mounted() {
        this.scanPorts();
        await this.getInitialSerialStatus();

        window.addEventListener('resize', this.resizePlot);

        this.$options.sockets.onmessage = (msg) => {
            const data = JSON.parse(msg.data);
            // console.log(data);

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
            if (!this.selectedPort) {
                ElMessage({
                    message: '请选择一个串口',
                    type: 'warning'
                });
                return;
            }

            const serialArgs = {
                opt: this.isConnected ? 'close' : 'open',
                port: this.selectedPort,
                baudrate: 115200,
                bytesize: 8,
                parity: 'N',
                stopbits: 1
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
            const layout = {
                xaxis: {
                    title: 'x (m)',
                    range: [-1.5, 3.5]
                },
                yaxis: {
                    title: 'y (m)',
                    range: [-0.5, 5.5]
                },
                dragmode: 'pan'
            };

            Plotly.newPlot(this.$refs.plotlyContainer, [], layout, {
                scrollZoom: true
            });
        },
        updatePlot(points) {
            const plotContainer = this.$refs.plotlyContainer;
            if (plotContainer) {
                const layout = plotContainer.layout || {};

                // 绘制点的数据
                const data = points.map(item => ('id' in item && 'position' in item) ? {
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
                    // showlegend: false
                } : null).filter(d => d !== null);

                // 绘制目标区域的圆形
                const targetAreas = points.map(item => {
                    if ('id' in item && 'target_position' in item && item.target_position) {
                        const [tx, ty] = item.target_position;
                        const targetColor = ('color' in item ? `rgba(${item.color[0]}, ${item.color[1]}, ${item.color[2]}, 0.3)` : 'rgba(128, 128, 128, 0.3)');
                        const borderColor = ('color' in item ? `rgba(${item.color[0]}, ${item.color[1]}, ${item.color[2]}, 0.6)` : 'rgba(128, 128, 128, 0.6)');

                        // 计算圆形区域的点
                        const theta = Array.from({ length: 36 }, (_, i) => i * (2 * Math.PI / 36));
                        const xCircle = theta.map(angle => tx + 0.3 * Math.cos(angle));
                        const yCircle = theta.map(angle => ty + 0.3 * Math.sin(angle));

                        return {
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
                        };
                    } else {
                        return null;
                    }
                }).filter(d => d !== null);

                // 绘制箭头的注释
                const annotations = points.map(item => {
                    if ('id' in item && 'position' in item && 'velocity' in item && item.velocity) {
                        const [x, y] = item.position;
                        const [vx, vy] = item.velocity;
                        const endX = x + vx;
                        const endY = y + vy;
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

                // 将箭头和目标区域分开处理
                const allData = [...data, ...targetAreas];

                // 更新图形
                Plotly.react(plotContainer, allData, layout);

                // 添加箭头注释
                Plotly.relayout(plotContainer, { annotations: annotations });
            }
        },
        resizePlot() {
            Plotly.Plots.resize(this.$refs.plotlyContainer);
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
    width: 60%;
    margin-right: 1rem;
}
.right-panel {
    width: 30%;
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
    font-weight: bold;
}
.property-value {
    flex: 1;
    color: #666;
    text-align: right;
}
</style>
