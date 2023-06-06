//the test case (js)
function test_sum(key, val) {
    function sum(n) {
        if (n < 2) {
            return n;
        }

        return n + sum(n - 1);
    }

    const result = sum(val);
    console.log(`${key}: ${result}`);
}

for (let i = 0; i <= 10; i++) {
    test_sum(i, i * 1000);
}