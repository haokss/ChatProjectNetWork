// 导入模块
const path = require('path')
const grpc = require('@grpc/grpc-js')
const protoLoader = require('@grpc/proto-loader')

// 获取路径
const PROTO_PATH = path.join(__dirname, 'message.proto');
const packageDefinition = protoLoader.loadSync(PROTO_PATH, {
    keepCase: true, longs: String, enums:String, defaults:true, oneofs:true
})

const protoDescriptor = grpc.loadPackageDefinition(packageDefinition)

const message_proto = protoDescriptor.message

// 导出模块
module.exports = message_proto