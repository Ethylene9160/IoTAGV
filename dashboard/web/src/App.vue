<script setup>
import { RouterView, useRoute } from 'vue-router';
import Navbar from '@/components/NavigationBar.vue';

const route = useRoute();
</script>

<template>
    <div id="body_container">
        <Navbar v-if="!route.meta?.noNavbar" />
        <RouterView />
    </div>
</template>

<style>
@import "@/assets/main.css";
</style>
