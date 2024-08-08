<template>
    <nav>
        <div class="logo">
            <router-link to="/">贰叁叁</router-link>
        </div>
        <ul>
            <li><router-link to="/">控制台</router-link></li>
            <li><router-link to="/about">关于</router-link></li>
        </ul>
        <div></div>
    </nav>
</template>

<style>
nav {
    display: flex;
    align-items: center;
    justify-content: space-between;
    background-color: #f5f5f5;
    padding: 1rem 2rem;
    color: #333;
    height: 30px;
}

.logo {
    font-size: 1.2rem;
    font-weight: bolder;
    color: #939393;
}

ul {
    list-style: none;
    display: flex;
    margin: 0;
    padding: 0;
}

li {
    margin: 0 1rem;
}

a {
    text-decoration: none;
    color: #8c8c8c;
    font-size: 1rem;
    transition: color 0.3s;
}

a:hover {
    color: #5d5d5d;
}

.user-menu {
    display: flex;
    align-items: center;
}

select {
    margin-left: 0.5rem;
}
</style>
