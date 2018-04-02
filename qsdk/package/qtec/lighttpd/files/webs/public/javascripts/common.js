function scrolltotop() {
    $('html,body').animate({scrollTop: '0px'}, 800);
}
var countnums=(function(){
    var trim=function(strings){
        return (strings||"").replace(/^(\s|\u00A0)+|(\s|\u00A0)+$/g,"");//+表示匹配一次或多次，|表示或者，\s和\u00A0匹配空白字符，/^以……开头，$以……结尾，/g全局匹配,/i忽略大小写
    }
    return function(_str){
        // _str=trim(_str);   //去除字符串的左右两边空格
        var strlength=_str.length;
        if(!strlength){   //如果字符串长度为零，返回零
            return 0;
        }
        var chinese=_str.match(/[\u4e00-\u9fa5]/g); //匹配中文，match返回包含中文的数组
        if(!chinese){
            return strlength;
        }else{
            return strlength+(chinese.length)*2; //计算字符个数
        }
    }
})();
function length_check(m) {
    if(m.replace(/(^\s*)|(\s*$)/g,"").length == 0)
    {
        return -1;
    }
}
var spacecheck=function (m) {
    if((/\s/g).test(m)){
        return -1;
    }
}
function cn_test(m) {
    var buf = m;
    for (var h = 0; h < buf.length; h++) {
        var tst = buf.substring(h, h + 1);
        if (tst.charCodeAt(0) < 0 || tst.charCodeAt(0) > 255) {
            // var ss = '密码不支持中文字符';
            return -1;
        }
    }
}
function pppoe_check(m,n,k,j) {
    var a = m.length;
    if (a == 0) {
        errorShow(k, "宽带帐号不能为空");
        return -1
    }
    if (cn_test(m) == -1) {
        errorShow(k, "宽带帐号不支持中文字符");
        return -1
    }
    if (spacecheck(m) == -1) {
        errorShow(k, '宽带帐号不支持空格');
        return -1;
    }
    if (a < 4 || a > 20) {
        errorShow(k, "请输入4-20位宽带帐号");
        return -1
    }
    var b = n.length;
    if (b == 0) {
        errorShow(j, "宽带密码不能为空");
        return -1
    }
    if (spacecheck(n) == -1) {
        errorShow(j, '宽带密码不支持空格');
        return -1;
    }
    if (cn_test(n) == -1) {
        errorShow(j, "宽带密码不支持中文字符");
        return -1
    }
    if (b < 4 || b > 20) {
        errorShow(j, "请输入4-20位宽带密码");
        return -1
    }
}
var check_reg={
    "ip_reg":(/^((?:(?:25[0-5]|2[0-4]\d|((1\d{2})|([1-9]?\d)))\.){3}(?:25[0-5]|2[0-4]\d|((1\d{2})|([1-9]?\d))))$/),
    "mask_reg":(/^(254|252|248|240|224|192|128|0)\.0\.0\.0$|^(255\.(254|252|248|240|224|192|128|0)\.0\.0)$|^(255\.255\.(254|252|248|240|224|192|128|0)\.0)$|^(255\.255\.255\.(254|252|248|240|224|192|128|0))$/),
    "gateway_reg":(/^((?:(?:25[0-5]|2[0-4]\d|((1\d{2})|([1-9]?\d)))\.){3}(?:25[0-5]|2[0-4]\d|((1\d{2})|([1-9]?\d))))$/),
    "ssid_reg":(/^.{1,32}$/),
    "wifi_pwd_reg":(/^.{8,63}$/),
    "manage_pwd_reg":(/^.{4,63}$/),

}

function ipcommoncheck(n) {
    var iparr=n.split('.');
    if(iparr.length<4){
        hostserror('ip地址请输入完整');
        return;
    }
    for(var i=0;i<iparr.length;i++){
        if((/[^\d]+/g).test(iparr[i])){
            hostserror('第'+(i+1)+'段ip值存在非数字字符');
            return;
        }
        if(iparr[i].length>3){
            hostserror('第'+(i+1)+'段ip值长度超出3位');
            return;
        }
        if(iparr[i]>255 || iparr[i]<0){
            hostserror('第'+(i+1)+'段ip值只能在0-255之间');
            return;
        }
        if(iparr[i].length==0){
            hostserror('第'+(i+1)+'段ip值请输入完整');
            return;
        }
    }
}

function static_check(m,n) {
    if(m.ip.length == 0)
    {
        errorShow(n.ip,"ip地址不能为空");
        return -1;
    }
    if (spacecheck(m.ip) == -1) {
        errorShow(n.ip,'ip地址不支持空格');
        return -1;
    }
    var ip_check_result = check_reg.ip_reg.test(m.ip);
    if(!ip_check_result){
        errorShow(n.ip,'ip地址输入错误');
        return -1;
    }
    if(m.mask.length == 0)
    {
        errorShow(n.mask,"子网掩码不能为空");
        return -1;
    }
    if (spacecheck(m.mask) == -1) {
        errorShow(n.mask,'子网掩码不支持空格');
        return -1;
    }
    var mask_check_result = check_reg.mask_reg.test(m.mask);
    if(!mask_check_result){
        errorShow(n.mask,'子网掩码输入错误');
        return -1;
    }
    if(m.gateway.length == 0)
    {
        errorShow(n.gateway,"默认网关不能为空");
        return -1;
    }
    if (spacecheck(m.gateway) == -1) {
        errorShow(n.gateway,'默认网关不支持空格');
        return -1;
    }
    var gateway_check_result =check_reg.gateway_reg.test(m.gateway);
    if(!gateway_check_result){
        errorShow(n.gateway,'默认网关输入错误');
        return -1;
    }
    if(m.preferreddns.length==0)
    {
        errorShow(n.preferreddns,"首选DNS不能为空");
        return -1;
    }
    if (spacecheck(m.preferreddns) == -1) {
        errorShow(n.preferreddns,'首选DNS不支持空格');
        return -1;
    }
    if (m.preferreddns) {
        if (!check_reg.ip_reg.test(m.preferreddns)) {
            errorShow(n.preferreddns,'首选DNS输入错误');
            return -1;
        }
    }
    if (m.sparedns) {
        if (spacecheck(m.sparedns) == -1) {
            errorShow(n.sparedns,'备用DNS不支持空格');
            return -1;
        }
        if (!check_reg.ip_reg.test(m.sparedns)) {
            errorShow(n.sparedns, '备用DNS输入错误');
            return -1;
        }
    }
    if(m.ip==m.gateway){
        errorShow('.ip-item',"ip地址不能默认网关相同");
        return -1;
    }
    var testip=m.ip.split('.');
    var testmask=m.mask.split('.');
    var testgateway=m.gateway.split('.');
    for(var i=0;i<3;i++){
        var a=parseInt(testip[i]) & parseInt(testmask[i]);
        var b=parseInt(testmask[i]) & parseInt(testgateway[i]);
        if(a!=b){
            errorShow(n.ip, 'ip地址与默认网关应在同一网段');
            return -1;
        }
    }
    var dns='';
    if(m.sparedns){
        dns=m.preferreddns+' '+m.sparedns;
    }else {
        if(m.preferreddns){
            dns=m.preferreddns;
        }else {
            dns='';
        }
    }
    return dns;
}
function errorShow(m,n) {
    $('.error-info').html('')
    $('.error-info').removeClass('error-color');
    var i=($(m).position().top).toFixed(0);
    var j=($(m).position().left+$(m).outerWidth()+6).toFixed(0);
    var k=($(m).parent().height()).toFixed(0);
    $('.error-tip').attr('style','visibility: visible;top:'+i+'px;left:'+j+'px;height:'+k+'px;line-height:'+k+'px;')
    setTimeout(function () {
        $('.error-info').html(n)
        $('.error-info').addClass('error-color');
        setTimeout(function () {
            $('.error-info').removeClass('error-color');
            setTimeout(function () {
                $('.error-info').addClass('error-color');
            },180)
        },180)
    },150)
}
$('input').on('input',function () {
    if($('.error-info').html().length>0){
        errorhide();
    }
})
$('input[type=checkbox] ').click(function () {
    if($(this).attr('checked')){
        $(this).removeAttr('checked');
    }else {
        $(this).attr('checked',true);
    }
})
function errorhide() {
    $('.error-info').html('')
    $('.error-tip').removeAttr('style')
}

postAjax=function (data,successfunc,errorfunc) {
    var success=function(res) {
        if(res.errorcode==0){
            successfunc(res);
        }else {
            err.error_msg(res);
        }
    }
    $.ajax({
        type:'POST',
        url:'/cgi-bin/web.cgi/',
        data:JSON.stringify(data),
        dataType:'json',
        success:success,
        error:errorfunc
    })
}
$( document ).ajaxSuccess(function( event, request, settings ) {
    // console.log(request)
    if (request.responseJSON && request.responseJSON.errorcode == '-100') {
        window.localStorage.removeItem('qtec_router_token');
        location.href='./login.html?redirect='+location.pathname+'?t='+Math.random()+location.hash;
    }
    if (request.responseJSON && request.responseJSON.errorcode == '-87') {
        window.localStorage.removeItem('qtec_router_token');
        location.href='./';
    }
});

$( document ).ajaxError(function( event, request, settings ) {
    // console.log(request);
    if ((request.errorcode == 400 || request.status ==0 ) && (location.hash!='#firmware-update' || location.hash!='#wireless-repeater' || location.pathname!='/home.html')){
        // $('.loading').attr('style','display:none;')
        resultTipshow(0,'网络异常，请检查');
    }
});