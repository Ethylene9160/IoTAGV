<template>
    <el-card class="collapsible-card">
        <template #header>
            <div slot="header" class="header-content">
                <span>{{ title }}</span>
                <el-button @click="togglePanel" type="text">{{ collapsed ? '展开' : '折叠' }}</el-button>
            </div>
        </template>
        <div v-show="isOpen" class="content">
            <slot></slot>
        </div>
    </el-card>
</template>

<script>
export default {
    props: {
        title: {
            type: String,
            required: true
        },
        defaultOpen: {
            type: Boolean,
            default: true
        }
    },
    data() {
        return {
            isOpen: this.defaultOpen
        };
    },
    methods: {
        togglePanel() {
            this.isOpen = !this.isOpen;
        }
    }
};
</script>

<style>
.collapsible-card {
    margin-bottom: 1rem;
}
.header-content {
    display: flex;
    justify-content: space-between;
    align-items: center;
}
.content {
    margin-top: 1rem;
}
</style>
